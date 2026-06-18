#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"
#include <IconsMaterialDesignIcons.h>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <WICTextureLoader.h>
#include <Utility/StringUtility.h>
#include <discord_rpc.h>
#include "Connection/PipeClient.h"
#include "Connection/SharedMemoryClient.h"
#include "Editor.h"
#include "Registry/EnumRegistry.h"
#include "Registry/PropertyRegistry.h"
#include "Registry/ResourceIDRegistry.h"
#include "Registry/ResourceInfoRegistry.h"
#include "Registry/TextListHashRegistry.h"
#include "Registry/TypeRegistry.h"
#include "Timer.h"
#include "UI/Documents/LibraryInfoDocument.h"
#include "UI/Documents/SceneDocument.h"
#include "UI/Panels/HashMapSearchPanel.h"
#include "UI/Panels/HeaderLibrariesSearchPanel.h"
#include "Utility/FileDialog.h"
#include "Utility/StringUtility.h"
Editor::Editor() {
  hwnd = nullptr;
  lastActiveDocument = nullptr;
  floatingToolsOnlyVisibleWhenParentIsFocused = true;
  directXRenderer = std::make_shared<DirectXRenderer>();
  imGuiRenderer = std::make_shared<ImGuiRenderer>();
  topLevelEditorWindowClass.ClassId = ImHashStr("TOPLEVEL_EDITOR", 0);
  topLevelEditorWindowClass.ViewportFlagsOverrideSet =
      ImGuiViewportFlags_NoAutoMerge;
  topLevelEditorWindowClass.ViewportFlagsOverrideClear =
      ImGuiViewportFlags_NoDecoration | ImGuiViewportFlags_NoTaskBarIcon;
  topLevelEditorWindowClass.ParentViewportId = 0;
  topLevelEditorWindowClass.DockingAllowUnclassed = false;
  topLevelEditorWindowClass.DockingAlwaysTabBar = true;
}
Editor::~Editor() {
  Discord_Shutdown();
  imGuiRenderer->Cleanup();
  directXRenderer->CleanupD3DDevice();
  DestroyWindow(hwnd);
  UnregisterClassA(wc.lpszClassName, wc.hInstance);
  CoUninitialize();
}
Editor &Editor::GetInstance() {
  static Editor instance;
  return instance;
}
bool Editor::Setup() {
  HICON hIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(101));
  wc = {sizeof(wc),
        CS_CLASSDC,
        WndProc,
        0L,
        0L,
        GetModuleHandle(nullptr),
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        "Hitman Absolution Editor",
        nullptr};
  wc.hIcon = hIcon;
  wc.hIconSm = hIcon;
  RegisterClassExA(&wc);
  int width = GetSystemMetrics(SM_CXSCREEN);
  int height = GetSystemMetrics(SM_CYSCREEN);
  hwnd = CreateWindowExA(0, wc.lpszClassName, "Hitman Absolution Editor",
                         WS_OVERLAPPEDWINDOW, 0, 0, width, height, nullptr,
                         nullptr, wc.hInstance, nullptr);
  Logger &logger = Logger::GetInstance();
  if (!directXRenderer->Setup(hwnd, &wc)) {
    logger.Log(Logger::Level::Error, "Failed to setup DirectX renderer!");
    return false;
  }
  ShowWindow(hwnd, SW_MAXIMIZE);
  UpdateWindow(hwnd);
  if (!imGuiRenderer->Setup(hwnd)) {
    logger.Log(Logger::Level::Error, "Failed to setup ImGui renderer!");
    return false;
  }
  Timer::Initialize();
  std::shared_ptr<SceneDocument> sceneDocument =
      std::make_shared<SceneDocument>("Scene", ICON_MDI_TERRAIN,
                                      Document::Type::Scene);
  documents.push_back(sceneDocument);
  Settings &settings = Settings::GetInstance();
  settings.LoadSettings();
  std::thread thread(&Editor::LoadRegistries, this);
  thread.detach();
  HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  if (FAILED(result)) {
    logger.Log(Logger::Level::Error, "Failed to initialize COM!");
    return false;
  }
  DiscordEventHandlers handlers;
  memset(&handlers, 0, sizeof(handlers));
  Discord_Initialize("1517179682892677160", &handlers, 1, nullptr);
  Logger::GetInstance().Log(Logger::Level::Info, "Editor successfully set up.");
  return true;
}
void Editor::Start() {
  Timer::SetFPSLimit();
  bool quit = false;
  Settings& settings = Settings::GetInstance();
  if (!settings.GetBackgroundImagePath().empty()) {
      LoadBackgroundImage(settings.GetBackgroundImagePath());
  }
  while (true) {
    Timer::Tick();
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      if (msg.message == WM_QUIT) {
        quit = true;
      }
    }
    if (quit) {
      break;
    }
    if (resizeWidth != 0 && resizeHeight != 0) {
      directXRenderer->GetSwapChain()->Resize(resizeWidth, resizeHeight);
      resizeWidth = 0;
      resizeHeight = 0;
    }
    UpdateDiscordPresence();
    Discord_RunCallbacks();
    Render();
  }
}
void Editor::Render() {
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
  RenderContent();
  ImGui::Render();
  directXRenderer->ClearBackBuffer();
  directXRenderer->SetBackBufferAsRenderTarget();
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
  }
  directXRenderer->GetSwapChain()->Present();
}
void Editor::RenderContent() {
  isSettingsPanelFocused = false;
  ImGuiWindowFlags windowFlags =
      ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
  windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
  windowFlags |=
      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
  ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->Pos);
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGui::SetNextWindowSize(viewport->Size);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  if (backgroundTextureSRV) {
      ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
  }
  ImGui::Begin("###DocumentRoot", nullptr, windowFlags);
  ImGui::PopStyleVar(3);
  if (backgroundTextureSRV) {
      ImGui::PopStyleColor();
  }
  ImGuiID rootDockspaceID = ImGui::GetID("RootDockspace");
  ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_NoCloseButton | ImGuiDockNodeFlags_NoSplit;
  if (backgroundTextureSRV) {
      dockspaceFlags |= ImGuiDockNodeFlags_PassthruCentralNode;
  }
  ImGui::DockSpace(rootDockspaceID, ImVec2(0, 0), dockspaceFlags, &topLevelEditorWindowClass);
  if (backgroundTextureSRV) {
      ImGui::GetBackgroundDrawList()->AddImage((void*)backgroundTextureSRV, viewport->Pos, ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255));
      ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 0.5f;
  } else {
      ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 1.0f;
  }
  for (size_t i = 0; i < documents.size(); ++i) {
    if (!*documents[i]->GetOpen()) {
      continue;
    }
    const bool isLastFocusedDocument = lastActiveDocument == documents[i];
    if (isLastFocusedDocument) {
      documents[i]->RenderMenuBar();
    }
    UpdateDocumentLocation(documents[i], rootDockspaceID);
  }
  for (size_t i = 0; i < documents.size(); ++i) {
    std::shared_ptr<Document> document = documents[i];
    if (!*document->GetOpen()) {
      if (lastActiveDocument == document) {
        lastActiveDocument = nullptr;
      }
      documents.erase(documents.begin() + i);
      i--;
    } else {
      UpdateDocumentContents(document);
    }
  }
  Settings &settings = Settings::GetInstance();
  if (settings.GetRuntimeFolderPath().empty()) {
    ImGui::OpenPopup("Runtime Folder Path");
  }
  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  if (ImGui::BeginPopupModal("Runtime Folder Path", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    static std::string runtimeFolderPath;
    ImGui::PushFont(Editor::GetInstance().GetImGuiRenderer()->GetMiddleFont());
    ImGui::TextUnformatted("Runtime Folder Path");
    ImGui::InputText("##RuntimeFolderPath", &runtimeFolderPath);
    ImGui::SameLine();
    if (ImGui::Button(ICON_MDI_FOLDER)) {
      runtimeFolderPath = FileDialog::OpenFolder();
    }
    ImGui::Separator();
    ImGui::SetItemDefaultFocus();
    ImGui::BeginDisabled(runtimeFolderPath.empty());
    if (ImGui::Button("Ok", ImVec2(120, 0))) {
      settings.SetRuntimeFolderPath(runtimeFolderPath);
      settings.UpdateIniFile("RuntimeFolderPath", runtimeFolderPath);
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();
    ImGui::PopFont();
    ImGui::EndPopup();
  }
  ImGui::End();
}
LRESULT Editor::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (SceneViewportPanel::IsWindowHovered() && msg == WM_MOUSEMOVE) {
    SceneViewportPanel::OnMouseMove();
  }
  if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) {
    return true;
  }
  switch (msg) {
  case WM_SIZE: {
    if (wParam == SIZE_MINIMIZED) {
      return 0;
    }
    resizeWidth = (UINT)LOWORD(lParam);
    resizeHeight = (UINT)HIWORD(lParam);
    return 0;
  }
  case WM_SYSCOMMAND:
    if ((wParam & 0xfff0) == SC_KEYMENU)
      return 0;
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  case WM_DPICHANGED: {
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports) {
      const RECT *suggested_rect = reinterpret_cast<RECT *>(lParam);
      SetWindowPos(hwnd, nullptr, suggested_rect->left, suggested_rect->top,
                   suggested_rect->right - suggested_rect->left,
                   suggested_rect->bottom - suggested_rect->top,
                   SWP_NOZORDER | SWP_NOACTIVATE);
    }
    break;
  }
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}
void Editor::LoadRegistries() {
  ResourceIDRegistry &resourceIDRegistry = ResourceIDRegistry::GetInstance();
  TypeRegistry &typeRegistry = TypeRegistry::GetInstance();
  ResourceInfoRegistry &resourceInfoRegistry =
      ResourceInfoRegistry::GetInstance();
  PropertyRegistry &propertyRegistry = PropertyRegistry::GetInstance();
  EnumRegistry &enumRegistry = EnumRegistry::GetInstance();
  TextListHashRegistry &textListHashRegistry =
      TextListHashRegistry::GetInstance();
  resourceIDRegistry.Load();
  typeRegistry.Load();
  resourceInfoRegistry.Load();
  propertyRegistry.Load();
  enumRegistry.Load();
  textListHashRegistry.Load();
}
static const char* GetDocumentExtension(Document::Type type) {
    switch (type) {
        case Document::Type::CppEntity: return ".CPPT";
        case Document::Type::CppEntityBlueprint: return ".CBLU";
        case Document::Type::TemplateEntity: return ".TEMP";
        case Document::Type::Texture: return ".TEXT";
        case Document::Type::TextList: return ".TELI";
        case Document::Type::Localization: return ".LOCR";
        case Document::Type::MultiLanguage: return ".LOCM";
        case Document::Type::RenderMaterialEntityType: return ".MATT";
        case Document::Type::RenderMaterialInstance: return ".MATI";
        case Document::Type::RenderMaterialEffect: return ".MATE";
        case Document::Type::SoundBlendContainerExternalParametersType: return ".SBPD";
        case Document::Type::SoundBlendContainerExternalParametersBlueprint: return ".SBPB";
        case Document::Type::WaveBankFSBF: return ".FSBF";
        case Document::Type::WaveBankFSBM: return ".FSBM";
        case Document::Type::WaveBankFSBS: return ".FSBS";
        case Document::Type::WaveBank: return ".WAVB";
        case Document::Type::WaveBankFX: return ".WBFX";
        case Document::Type::FlashMovie: return ".SWFF";
        case Document::Type::GFXMovie: return ".GFXF";
        case Document::Type::AnimationDatabase: return ".ChrT";
        case Document::Type::SoundDefinitions: return ".SDEF";
        case Document::Type::RenderPrimitive: return ".PRIM";
        default: return "";
    }
}
void Editor::UpdateDiscordPresence() {
  static bool wasDiscordRPCEnabled = true;
  bool isDiscordRPCEnabled = Settings::GetInstance().GetEnableDiscordRPC();
  if (!isDiscordRPCEnabled) {
    if (wasDiscordRPCEnabled) {
      Discord_ClearPresence();
      wasDiscordRPCEnabled = false;
    }
    return;
  }
  static std::shared_ptr<Document> cachedActiveDocument = nullptr;
  static int64_t startTimestamp = std::time(nullptr);
  static size_t numDocuments = -1;
  static bool wasSettingsFocused = false;
  bool forceUpdate = false;
  if (!wasDiscordRPCEnabled) {
    forceUpdate = true;
    wasDiscordRPCEnabled = true;
  }
  if (isSettingsPanelFocused != wasSettingsFocused) {
    forceUpdate = true;
    wasSettingsFocused = isSettingsPanelFocused;
  }
  if (forceUpdate || cachedActiveDocument != lastActiveDocument ||
      numDocuments != documents.size()) {
    cachedActiveDocument = lastActiveDocument;
    numDocuments = documents.size();
    DiscordRichPresence discordPresence;
    memset(&discordPresence, 0, sizeof(discordPresence));
    static std::string stateString;
    if (isSettingsPanelFocused) {
      stateString = "Configuring Settings";
    } else if (lastActiveDocument) {
      std::string rawName = lastActiveDocument->GetRawName();
      if (rawName == "Scene") {
        stateString = "Dashboard";
      } else {
        const char *ext = GetDocumentExtension(lastActiveDocument->GetType());
        stateString = std::format("Editing {}{}", rawName, ext);
      }
    } else {
      stateString = "Dashboard";
    }
    discordPresence.state = stateString.c_str();
    discordPresence.startTimestamp = startTimestamp;
    Discord_UpdatePresence(&discordPresence);
  }
}
std::shared_ptr<DirectXRenderer> Editor::GetDirectXRenderer() const {
  return directXRenderer;
}
std::shared_ptr<ImGuiRenderer> Editor::GetImGuiRenderer() const {
  return imGuiRenderer;
}
std::vector<std::shared_ptr<Document>> &Editor::GetDocuments() {
  return documents;
}
std::shared_ptr<Document> Editor::GetLastActiveDocument() const {
  return lastActiveDocument;
}
void Editor::SetupLayout(std::shared_ptr<Document> document,
                         const ImGuiID dockspaceID,
                         const ImVec2 dockspaceSize) {
  document->ResetToolsVisibility();
  ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
  ImGui::DockBuilderSetNodeSize(dockspaceID, dockspaceSize);
  document->CreateLayout(dockspaceID, dockspaceSize);
}
void Editor::UpdateDocumentLocation(std::shared_ptr<Document> document,
                                    const ImGuiID toplevelDockspaceID) {
  IM_ASSERT(toplevelDockspaceID != 0);
  ImGui::SetNextWindowClass(&topLevelEditorWindowClass);
  if (document->GetDockID() != 0) {
    ImGui::SetNextWindowDockID(document->GetDockID());
    document->SetDockID(0);
  } else {
    ImGui::SetNextWindowDockID(toplevelDockspaceID, ImGuiCond_FirstUseEver);
  }
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None;
  bool *open = document->GetOpen();
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  bool visible;
  if (document->GetType() == Document::Type::Scene) {
    visible = ImGui::Begin(
        std::format("{}##{}", document->GetName(), document->GetID()).c_str(),
        nullptr, windowFlags);
  } else {
    visible = ImGui::Begin(
        std::format("{}##{}", document->GetName(), document->GetID()).c_str(),
        open, windowFlags);
  }
  ImGui::PopStyleVar();
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
    lastActiveDocument = document;
  ImGuiWindowClass *toolWindowsClass = document->GetToolWindowsClass();
  toolWindowsClass->ClassId = document->GetID();
  toolWindowsClass->ViewportFlagsOverrideSet =
      ImGuiViewportFlags_NoTaskBarIcon | ImGuiViewportFlags_NoDecoration;
  toolWindowsClass->ParentViewportId =
      ImGui::GetWindowViewport()
          ->ID;
  toolWindowsClass->DockingAllowUnclassed = true;
  toolWindowsClass->DockNodeFlagsOverrideSet =
      ImGuiDockNodeFlags_NoCloseButton;
  document->SetCurrentDockID(ImGui::GetWindowDockID());
  document->SetPreviousLocationID(document->GetCurrentLocationID());
  document->SetCurrentLocationID(document->GetCurrentDockID() != 0
                                     ? document->GetCurrentDockID()
                                     : document->GetID());
  document->SetPreviousDockspaceID(document->GetCurrentDockspaceID());
  document->SetCurrentDockspaceID(document->CalculateDockspaceID());
  IM_ASSERT(document->GetCurrentDockspaceID() != 0);
  ImGui::End();
}
void Editor::UpdateDocumentContents(std::shared_ptr<Document> document) {
  ImGui::Begin(
      std::format("{}##{}", document->GetName(), document->GetID()).c_str());
  if (ImGui::GetCurrentWindow()->BeginCount !=
      2)
  {
    ImGui::End();
    return;
  }
  const ImGuiID dockspaceID = document->GetCurrentDockspaceID();
  ImVec2 dockspaceSize = ImGui::GetContentRegionAvail();
  if (document->HasToolBar()) {
    const float toolbarHeight = ImGui::GetFrameHeightWithSpacing() +
                                ImGui::GetStyle().FramePadding.y * 2;
    dockspaceSize.y -= toolbarHeight + 1.f;
  }
  if (document->GetPreviousLocationID() != 0 &&
      document->GetPreviousLocationID() != document->GetCurrentLocationID()) {
    int previousDockspaceReferenceCount = 0;
    int currentDockspaceReferenceCount = 0;
    for (size_t i = 0; i < documents.size(); i++) {
      std::shared_ptr<Document> document2 = documents[i];
      if (document2->GetCurrentDockspaceID() ==
          document->GetPreviousDockspaceID()) {
        previousDockspaceReferenceCount++;
      }
      if (document2->GetCurrentDockspaceID() ==
          document->GetCurrentDockspaceID()) {
        currentDockspaceReferenceCount++;
      }
    }
#if EDITOR_CONFIG_ONMERGE_OVERWRITE_WITH_SOURCE_LAYOUT
    document->LayoutCopy(document->GetPreviousDockspaceID(),
                         document->GetCurrentDockspaceID());
#else
    if (currentDockspaceReferenceCount <= 1) {
      document->LayoutCopy(document->GetPreviousDockspaceID(),
                           document->GetCurrentDockspaceID());
    }
#endif
    if (previousDockspaceReferenceCount == 0) {
      ImGui::DockBuilderRemoveNode(document->GetPreviousDockspaceID());
      char windowSuffix[16];
      ImFormatString(windowSuffix, IM_ARRAYSIZE(windowSuffix), "##%08X",
                     document->GetPreviousDockspaceID());
      size_t windowSuffixLength = strlen(windowSuffix);
      ImGuiContext &g = *GImGui;
      for (ImGuiWindowSettings *settings = g.SettingsWindows.begin();
           settings != NULL;
           settings = g.SettingsWindows.next_chunk(settings)) {
        if (settings->ID == 0) {
          continue;
        }
        const char *windowName = settings->GetName();
        size_t windowNameLength = strlen(windowName);
        if (windowNameLength >= windowSuffixLength) {
          if (strcmp(windowName + windowNameLength - windowSuffixLength,
                     windowSuffix) == 0)
          {
            ImGui::ClearWindowSettings(windowName);
          }
        }
      }
    }
  } else if (ImGui::DockBuilderGetNode(document->GetCurrentDockspaceID()) ==
             nullptr) {
    SetupLayout(document, dockspaceID, dockspaceSize);
  }
  bool visible = true;
#if EDITOR_CONFIG_SAME_LOCATION_SHARE_LAYOUT
  if (ImGui::GetCurrentWindow()->Hidden) {
    visible = false;
  }
#endif
  if (!visible) {
    ImGui::DockSpace(dockspaceID, dockspaceSize,
                     ImGuiDockNodeFlags_KeepAliveOnly,
                     document->GetToolWindowsClass());
    ImGui::End();
    return;
  }
  if (document->HasToolBar()) {
    document->RenderToolBar();
  }
  ImGui::DockSpace(dockspaceID, dockspaceSize, ImGuiDockNodeFlags_None,
                   document->GetToolWindowsClass());
  ImGui::End();
  const bool isLastFocusedDocument = lastActiveDocument == document;
  document->RenderPanels(isLastFocusedDocument);
}
void Editor::LoadBackgroundImage(const std::string& path) {
    if (backgroundTextureSRV) {
        backgroundTextureSRV->Release();
        backgroundTextureSRV = nullptr;
    }
    std::wstring wpath = StringUtility::AnsiStringToWideString(path);
    ID3D11Resource* texture = nullptr;
    HRESULT hr = DirectX::CreateWICTextureFromFile(
        directXRenderer->GetD3D11Device(),
        directXRenderer->GetD3D11DeviceContext(),
        wpath.c_str(),
        &texture,
        &backgroundTextureSRV
    );
    if (FAILED(hr)) {
        Logger::GetInstance().Log(Logger::Level::Error, "Failed to load background image.");
    }
    if (texture) {
        texture->Release();
    }
}
void Editor::SetSettingsPanelFocused(bool focused) {
    isSettingsPanelFocused = focused;
}