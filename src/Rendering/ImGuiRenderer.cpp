#include "imgui_internal.h"
#include <IconsMaterialDesignIcons.h>
#include "Rendering/ImGuiRenderer.h"
#include "Allocation.h"
#include "Logger.h"
#include "Rendering/DirectXRenderer.h"
#include "Editor.h"
bool ImGuiRenderer::Setup(HWND hwnd)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    if (!ImGui_ImplWin32_Init(hwnd))
    {
        return false;
    }
    if (!ImGui_ImplDX11_Init(Editor::GetInstance().GetDirectXRenderer()->GetD3D11Device(), Editor::GetInstance().GetDirectXRenderer()->GetD3D11DeviceContext()))
    {
        return false;
    }
    SetStyle();
    SetFont();
    io.IniFilename = nullptr;
    Logger::GetInstance().Log(Logger::Level::Info, "ImGui renderer successfully set up.");
    return true;
}
void ImGuiRenderer::Cleanup()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}
void ImGuiRenderer::SetStyle()
{
    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;
    colors[ImGuiCol_Text] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.275f, 0.275f, 0.275f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.082f, 0.082f, 0.082f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.082f, 0.082f, 0.082f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.082f, 0.082f, 0.082f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.082f, 0.082f, 0.082f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 0.44f, 0.88f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.000f, 0.434f, 0.878f, 1.000f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.000f, 0.434f, 0.878f, 1.000f);
    colors[ImGuiCol_Button] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.000f, 0.439f, 0.878f, 0.824f);
    colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.082f, 0.082f, 0.082f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.082f, 0.082f, 0.082f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.19f, 0.53f, 0.78f, 0.22f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.00f, 0.44f, 0.88f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.00f, 0.47f, 0.94f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.00f, 0.44f, 0.88f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.00f, 0.47f, 0.94f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.197f, 0.197f, 0.197f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.197f, 0.197f, 0.197f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.188f, 0.529f, 0.780f, 1.000f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.00f, 0.44f, 0.88f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.00f, 0.44f, 0.88f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
    style->AntiAliasedFill = true;
    style->AntiAliasedLines = true;
    style->AntiAliasedLinesUseTex = true;
    style->WindowPadding = ImVec2(8.0f, 4.0f);
    style->FramePadding = ImVec2(4.0f, 6.0f);
    style->TabMinWidthForCloseButton = 0.1f;
    style->CellPadding = ImVec2(8.0f, 4.0f);
    style->ItemSpacing = ImVec2(8.0f, 3.0f);
    style->ItemInnerSpacing = ImVec2(2.0f, 4.0f);
    style->TouchExtraPadding = ImVec2(0.0f, 0.0f);
    style->IndentSpacing = 12;
    style->ScrollbarSize = 14;
    style->GrabMinSize = 10;
    style->WindowBorderSize = 1.0f;
    style->ChildBorderSize = 0.0f;
    style->PopupBorderSize = 1.5f;
    style->FrameBorderSize = 0.5f;
    style->TabBorderSize = 0.0f;
    style->WindowRounding = 6.0f;
    style->ChildRounding = 0.0f;
    style->FrameRounding = 2.0f;
    style->PopupRounding = 2.0f;
    style->ScrollbarRounding = 3.0f;
    style->GrabRounding = 2.0f;
    style->LogSliderDeadzone = 4.0f;
    style->TabRounding = 3.0f;
    style->WindowTitleAlign = ImVec2(0.0f, 0.5f);
    style->WindowMenuButtonPosition = ImGuiDir_None;
    style->ColorButtonPosition = ImGuiDir_Left;
    style->ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style->SelectableTextAlign = ImVec2(0.0f, 0.0f);
    style->DisplaySafeAreaPadding = ImVec2(8.0f, 8.0f);
    ImGuiColorEditFlags colorEditFlags = ImGuiColorEditFlags_AlphaBar
        | ImGuiColorEditFlags_AlphaPreviewHalf
        | ImGuiColorEditFlags_DisplayRGB
        | ImGuiColorEditFlags_InputRGB
        | ImGuiColorEditFlags_PickerHueBar
        | ImGuiColorEditFlags_Uint8;
    ImGui::SetColorEditOptions(colorEditFlags);
}
void ImGuiRenderer::AddIconFont(float fontSize)
{
    ImGuiIO& io = ImGui::GetIO();
    static constexpr ImWchar iconsRanges[] = { ICON_MIN_MDI, ICON_MAX_MDI, 0 };
    ImFontConfig iconsConfig;
    iconsConfig.MergeMode = true;
    iconsConfig.GlyphOffset = { 0.f, 2.f };
    static constexpr const char* materialIconsRegularFontPath = "assets/fonts/materialdesignicons-webfont.ttf";
    io.Fonts->AddFontFromFileTTF(materialIconsRegularFontPath, fontSize, &iconsConfig, iconsRanges);
}
void ImGuiRenderer::SetFont()
{
    ImGuiIO& io = ImGui::GetIO();
    float defaultFontSize = 22.0f;
    float middleFontSize = 20.0f;
    float smallFontSize = 16.0f;
    constexpr const char* regularFontPath = "assets/fonts/OpenSans-Regular.ttf";
    constexpr const char* boldFontPath = "assets/fonts/OpenSans-Bold.ttf";
    constexpr const char* italicFontPath = "assets/fonts/OpenSans-Italic.ttf";
    constexpr const char* consolasRegularFontPath = "assets/fonts/Consolas Regular.ttf";
    constexpr const char* consolasBoldFontPath = "assets/fonts/Consolas Bold.ttf";
    defaultFont = io.Fonts->AddFontFromFileTTF(regularFontPath, defaultFontSize, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    AddIconFont(defaultFontSize);
    middleFont = io.Fonts->AddFontFromFileTTF(regularFontPath, middleFontSize, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    AddIconFont(middleFontSize);
    smallFont = io.Fonts->AddFontFromFileTTF(regularFontPath, smallFontSize, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    AddIconFont(smallFontSize);
    boldFont = io.Fonts->AddFontFromFileTTF(boldFontPath, defaultFontSize, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    AddIconFont(defaultFontSize);
    middleitalicFont = io.Fonts->AddFontFromFileTTF(italicFontPath, middleFontSize, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    AddIconFont(middleFontSize);
    consolasRegularFont = io.Fonts->AddFontFromFileTTF(consolasRegularFontPath, middleFontSize, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    consolasBoldFont = io.Fonts->AddFontFromFileTTF(consolasBoldFontPath, middleFontSize, nullptr, io.Fonts->GetGlyphRangesCyrillic());
}
ImFont* ImGuiRenderer::GetDefaultFont()
{
    return defaultFont;
}
ImFont* ImGuiRenderer::GetMiddleFont()
{
    return middleFont;
}
ImFont* ImGuiRenderer::GetSmallFont()
{
    return smallFont;
}
ImFont* ImGuiRenderer::GetBoldFont()
{
    return boldFont;
}
ImFont* ImGuiRenderer::GetMiddleItalicFont()
{
    return middleitalicFont;
}
ImFont* ImGuiRenderer::GetConsolasRegularFont()
{
    return consolasRegularFont;
}
ImFont* ImGuiRenderer::GetConsolasBoldFont()
{
    return consolasBoldFont;
}