#include <format>
#include <fstream>
#include <thread>
#include <filesystem>

#include <IconsMaterialDesignIcons.h>

#include "UI/Panels/ResourceBrowserPanel.h"
#include "Utility/StringUtility.h"
#include "Logger.h"
#include "Utility/ResourceUtility.h"
#include "Utility/UI.h"
#include "Utility/FileDialog.h"
#include "Utility/ResourcePatcher.h"
#include "UI/Panels/HexViewerPanel.h"
#include "UI/Panels/ResourceInfoPanel.h"
#include "UI/Panels/SceneHierarchyPanel2.h"
#include "UI/Panels/ComponentPropertiesPanel.h"
#include "UI/Panels/BoneHierarchyPanel.h"
#include "Registry/ResourceInfoRegistry.h"
#include "Registry/ResourceIDRegistry.h"
#include "Utility/TaskRunner.h"
#include "UI/Documents/TemplateEntityDocument.h"
#include "UI/Documents/TextureDocument.h"
#include "UI/Documents/CppEntityDocument.h"
#include "UI/Documents/CppEntityBlueprintDocument.h"
#include "UI/Documents/TextListDocument.h"
#include "UI/Documents/LocalizationDocument.h"
#include "UI/Documents/MultiLanguageDocument.h"
#include "UI/Documents/RenderMaterialEntityTypeDocument.h"
#include "UI/Documents/RenderMaterialInstanceDocument.h"
#include "UI/Documents/RenderMaterialEffectDocument.h"
#include "UI/Documents/SoundBlendContainerExternalParametersTypeDocument.h"
#include "UI/Documents/SoundBlendContainerExternalParametersBlueprintDocument.h"
#include "UI/Documents/WaveBankFSBFDocument.h"
#include "UI/Documents/WaveBankFSBMDocument.h"
#include "UI/Documents/WaveBankFSBSDocument.h"
#include "UI/Documents/WaveBankDocument.h"
#include "UI/Documents/WaveBankFXDocument.h"
#include "UI/Documents/FlashMovieDocument.h"
#include "UI/Documents/GFXMovieDocument.h"
#include "UI/Documents/AnimationDatabaseDocument.h"
#include "UI/Documents/SoundDefinitionsDocument.h"
#include "UI/Documents/GlobalResourceIndexDocument.h"
#include "UI/Documents/BehaviorTreeEntityBlueprintDocument.h"
#include "UI/Documents/CompositionBlueprintDocument.h"
#include "UI/Documents/PackageListDocument.h"
#include "UI/Documents/ScatterDataDocument.h"
#include "UI/Documents/FabricResourceEntityDocument.h"
#include "UI/Documents/RenderPrimitiveDocument.h"
#include "UI/Documents/ClothDocument.h"
#include "UI/Documents/BoneRigDocument.h"
#include "UI/Documents/PhysicsDocument.h"
#include "Resources/TextList.h"
#include "Resources/Localization.h"
#include "Resources/MultiLanguage.h"
#include "Editor.h"

ResourceBrowserPanel::ResourceBrowserPanel(const char* name, const char* icon) : BasePanel(name, icon)
{
    isInputTextActive = false;
    selectedNodeIndex = -1;
    showResourceExportPopup = false;
    showBatchExportPopup = false;
    showBatchImportPopup = false;

    LoadResourceTypes();
    AddRootResourceNodes();
}

ResourceBrowserPanel::~ResourceBrowserPanel()
{
}

void ResourceBrowserPanel::Render()
{
    if (!Begin())
    {
        return;
    }

    ImGui::PushFont(Editor::GetInstance().GetImGuiRenderer()->GetMiddleFont());

    const ImVec2 framePadding = ImGui::GetStyle().FramePadding;
    float itemWidth = ImGui::GetContentRegionAvail().x - (UI::GetIconButtonSize("  " ICON_MDI_FILTER, "").x + 2.0f * framePadding.x);

    ImGui::PushItemWidth(itemWidth);

    if (ImGui::InputTextWithHint("##SearchResource", "Search Resource...", resourceName, IM_ARRAYSIZE(resourceName)))
    {
        assemblyNode.children.clear();
        modulesNode.children.clear();

        AddRootResourceNodes();
    }

    ImGui::PopItemWidth();

    isInputTextActive = ImGui::IsItemActive();

    if (isInputTextActive &&
        (!(assemblyNode.children.size() == 1 && assemblyNode.children[0].name.empty()) ||
            !(modulesNode.children.size() == 1 && modulesNode.children[0].name.empty())))
    {
        assemblyNode.children.clear();
        modulesNode.children.clear();

        AddRootResourceNodes();
    }

    ImGui::SameLine();

    if (UI::IconButton("  " ICON_MDI_FILTER, ""))
    {
        ImGui::OpenPopup("Filter Resources");
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();

    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Filter Resources", nullptr))
    {
        ImGui::Text("Resource Types");
        ImGui::Spacing();

        ImGui::BeginChild("ScrollingRegion", ImVec2(300, 300));

        for (auto it = resourceTypes.begin(); it != resourceTypes.end(); ++it)
        {
            ImGui::Checkbox(it->first.c_str(), &it->second);
        }

        ImGui::EndChild();
        ImGui::Spacing();
        ImGui::Spacing();

        static bool selectAll = true;

        if (ImGui::Checkbox("Select All", &selectAll))
        {
            for (auto it = resourceTypes.begin(); it != resourceTypes.end(); ++it)
            {
                it->second = selectAll;
            }
        }

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        if (UI::IconButton("  " ICON_MDI_CHECK, " Ok "))
        {
            ImGui::CloseCurrentPopup();

            assemblyNode.children.clear();
            modulesNode.children.clear();

            AddRootResourceNodes();
        }

        ImGui::EndPopup();
    }

    ImGui::Spacing();

    if (ResourceInfoRegistry::GetInstance().IsResourcesInfoLoaded())
    {
        RenderTree(assemblyNode, assemblyNode.name);
        RenderTree(modulesNode, modulesNode.name);

        UI::ResourceExportPopup(showResourceExportPopup, resource);

        RenderBatchExportPopup();
        RenderBatchImportPopup();
    }

    ImGui::PopFont();

    End();
}

void ResourceBrowserPanel::RenderTree(ResourceNode& parentNode, std::string parentPath)
{
    ImGui::PushID(&parentNode);

    std::string label;
    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_SpanFullWidth;
    const bool isNodeSelected = selectedNodeIndex == parentNode.index;

    if (isNodeSelected)
    {
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
    }

    if (isInputTextActive)
    {
        ImGui::SetNextItemOpen(false);
    }

    if (parentNode.children.size() > 0)
    {
        label = std::format("{} {}", ICON_MDI_FOLDER, parentNode.name);
        nodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;

        bool isNodeOpen = ImGui::TreeNodeEx(label.c_str(), nodeFlags);

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        {
            selectedNodeIndex = parentNode.index;
        }

        if (ImGui::BeginPopupContextItem())
        {
            RenderFolderContextMenu(parentNode, parentPath);

            ImGui::EndPopup();
        }

        if (isNodeOpen)
        {
            if (parentNode.children.size() == 1 && parentNode.children[0].name.empty())
            {
                AddChildren(parentNode, parentPath);
            }

            for (ResourceNode& resourceNode : parentNode.children)
            {
                if (parentPath.ends_with("/"))
                {
                    RenderTree(resourceNode, parentPath + resourceNode.name);
                }
                else
                {
                    RenderTree(resourceNode, parentPath + "/" + resourceNode.name);
                }
            }

            ImGui::TreePop();
        }
    }
    else
    {
        label = std::format("{} {}", ICON_MDI_FILE, parentNode.name);
        nodeFlags |= ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf;

        ImGui::TreeNodeEx(label.c_str(), nodeFlags);

        if ((ImGui::IsItemClicked() || ImGui::IsItemClicked(ImGuiPopupFlags_MouseButtonRight)) && !ImGui::IsItemToggledOpen())
        {
            selectedNodeIndex = parentNode.index;
        }

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
        {
            CreateResourceDocument(parentNode);
        }

        if (ImGui::BeginPopupContextItem())
        {
            RenderContextMenu(parentNode);

            ImGui::EndPopup();
        }
    }

    ImGui::PopID();
}

void ResourceBrowserPanel::RenderContextMenu(ResourceNode& resourceNode)
{
    static std::string exportResourceLabel = std::format("{} Export Resource", ICON_MDI_EXPORT);

    if (ImGui::MenuItem(exportResourceLabel.c_str()))
    {
        showResourceExportPopup = true;

        const ResourceInfoRegistry::ResourceInfo& resourceInfo = ResourceInfoRegistry::GetInstance().GetResourceInfo(resourceNode.hash);
        resource = ResourceUtility::CreateResource(resourceInfo.type);
        std::string resourceName = ResourceUtility::GetResourceName(resourceInfo.resourceID);

        resource->SetHash(resourceInfo.hash);
        resource->SetResourceID(resourceInfo.resourceID);
        resource->SetHeaderLibraries(&resourceInfo.headerLibraries);
        resource->SetName(resourceName);

        std::thread thread(&ResourceBrowserPanel::LoadResource, this, resource, resourceNode, true);

        thread.detach();
    }

    const ResourceInfoRegistry::ResourceInfo& resInfo = ResourceInfoRegistry::GetInstance().GetResourceInfo(resourceNode.hash);
    if (resInfo.type == "TELI" || resInfo.type == "LOCR" || resInfo.type == "LOCM")
    {
        static std::string importResourceLabel = std::format("{} Import JSON (Replace Localization)", ICON_MDI_IMPORT);

        if (ImGui::MenuItem(importResourceLabel.c_str()))
        {
            std::string jsonPath = FileDialog::OpenFile("JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0");
            if (!jsonPath.empty())
            {
                std::shared_ptr<Resource> importResource = ResourceUtility::CreateResource(resInfo.type);
                std::string resName = ResourceUtility::GetResourceName(resInfo.resourceID);

                importResource->SetHash(resInfo.hash);
                importResource->SetResourceID(resInfo.resourceID);
                importResource->SetHeaderLibraries(&resInfo.headerLibraries);
                importResource->SetName(resName);

                ResourceUtility::LoadResource(importResource);

                if (resInfo.type == "TELI")
                {
                    std::shared_ptr<TextList> textList = std::static_pointer_cast<TextList>(importResource);
                    textList->Deserialize();
                    textList->ImportFromJson(jsonPath);
                }
                else if (resInfo.type == "LOCR")
                {
                    std::shared_ptr<Localization> localization = std::static_pointer_cast<Localization>(importResource);
                    localization->Deserialize();
                    localization->ImportFromJson(jsonPath);
                }
                else if (resInfo.type == "LOCM")
                {
                    std::shared_ptr<MultiLanguage> multiLanguage = std::static_pointer_cast<MultiLanguage>(importResource);
                    multiLanguage->Deserialize();
                    multiLanguage->ImportFromJson(jsonPath);
                }

                std::string savePath = FileDialog::SaveFile("RAW Files (*.raw)\0*.raw\0All Files (*.*)\0*.*\0");
                if (!savePath.empty())
                {
                    importResource->ExportRawData(savePath);
                }
            }
        }

        static std::string patchResourceLabel = std::format("{} Patch Resource Library (Write Back)", ICON_MDI_FILE_REPLACE);

        if (ImGui::MenuItem(patchResourceLabel.c_str()))
        {
            std::string jsonPath = FileDialog::OpenFile("JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0");
            if (!jsonPath.empty())
            {
                std::shared_ptr<Resource> importResource = ResourceUtility::CreateResource(resInfo.type);
                std::string resName = ResourceUtility::GetResourceName(resInfo.resourceID);

                importResource->SetHash(resInfo.hash);
                importResource->SetResourceID(resInfo.resourceID);
                importResource->SetHeaderLibraries(&resInfo.headerLibraries);
                importResource->SetName(resName);

                // Fully load the resource to get the correct offsets from the header library
                if (resInfo.headerLibraries.size() > 0)
                {
                    importResource->LoadResource(0, resInfo.headerLibraries[0].chunkIndex, resInfo.headerLibraries[0].indexInLibrary, true, false, true);
                }
                else
                {
                    importResource->LoadResource(0, -1, -1, true, false, true);
                }

                if (resInfo.type == "TELI")
                {
                    std::shared_ptr<TextList> textList = std::static_pointer_cast<TextList>(importResource);
                    textList->Deserialize();
                    textList->ImportFromJson(jsonPath);
                    textList->SerializeToBuffer();
                }
                else if (resInfo.type == "LOCR")
                {
                    std::shared_ptr<Localization> localization = std::static_pointer_cast<Localization>(importResource);
                    localization->Deserialize();
                    localization->ImportFromJson(jsonPath);
                    localization->SerializeToBuffer();
                }
                else if (resInfo.type == "LOCM")
                {
                    std::shared_ptr<MultiLanguage> multiLanguage = std::static_pointer_cast<MultiLanguage>(importResource);
                    multiLanguage->Deserialize();
                    multiLanguage->ImportFromJson(jsonPath);
                    multiLanguage->SerializeToBuffer();
                }

                std::string resourceLibPath = importResource->GetResourceLibraryFilePath();
                std::string headerLibPath = importResource->GetHeaderLibraryFilePath();
                unsigned int offsetInResLib = importResource->GetOffsetInResourceLibrary();
                unsigned int offsetInHeaderLib = importResource->GetOffsetInHeaderLibrary();
                const void* newData = importResource->GetResourceData();
                unsigned int newDataSize = importResource->GetResourceDataSize();

                if (ResourcePatcher::PatchResourceLibrary(resourceLibPath, headerLibPath, offsetInResLib, offsetInHeaderLib, newData, newDataSize))
                {
                    Logger::GetInstance().Log(Logger::Level::Info, "Resource patched successfully.");
                }
                else
                {
                    Logger::GetInstance().Log(Logger::Level::Error, "Failed to patch resource.");
                }
            }
        }
    }
}

void ResourceBrowserPanel::AddChildren(ResourceNode& parentNode, const std::string& parentPath)
{
    parentNode.children.clear();

    std::unordered_map<unsigned long long, ResourceInfoRegistry::ResourceInfo> resourcesInfo = ResourceInfoRegistry::GetInstance().GetResourcesInfo();
    std::map<std::string, ResourceNode> resourceNodes;

    for (auto it = resourcesInfo.begin(); it != resourcesInfo.end(); ++it)
    {
        ResourceInfoRegistry::ResourceInfo& resourceInfo = it->second;

        //Skip assembly paths if parent is module
        if (parentPath.starts_with("m") && resourceInfo.resourceID.starts_with("[a"))
        {
            continue;
        }

        if (!resourceTypes[resourceInfo.type] || !resourceInfo.resourceID.contains(resourceName))
        {
            continue;
        }

        if (resourceInfo.resourceID.starts_with("[" + parentPath) || resourceInfo.resourceID.starts_with("[[" + parentPath)) //[[ is case with MATE resources
        {
            const size_t index = resourceInfo.resourceID.find(parentPath);
            std::string pathPart = resourceInfo.resourceID.substr(index + parentPath.length());

            ResourceNode resourceNode;
            bool isFolder = StringUtility::Count(pathPart, '/') > 0;
            bool isAspectEntityResource = false;

            if (pathPart.starts_with("/"))
            {
                const size_t index2 = pathPart.find('/', 1);

                if (index2 != -1)
                {
                    pathPart = pathPart.substr(1, pathPart.find('/', 1) - 1);
                }
                else
                {
                    pathPart = pathPart.substr(1);
                    isFolder = false;
                }
            }
            else
            {
                if (parentPath.starts_with("m"))
                {
                    if (pathPart.contains('?'))
                    {
                        pathPart = pathPart.substr(0, pathPart.find('?') + 1);
                        isFolder = true;
                    }
                }
                else
                {
                    if (pathPart.starts_with("](["))
                    {
                        std::string references = resourceInfo.resourceID.substr(resourceInfo.resourceID.find("(") + 1, resourceInfo.resourceID.find(")") - resourceInfo.resourceID.find("(") - 1);
                        std::vector<std::string> references2 = StringUtility::Split(references, ',');
                        const size_t referenceCount = references2.size();

                        pathPart = "";

                        for (size_t i = 0; i < referenceCount; ++i)
                        {
                            pathPart += ResourceUtility::GetResourceName(references2[i]);

                            if (i < referenceCount - 1)
                            {
                                pathPart += ", ";
                            }
                        }

                        isFolder = false;
                        isAspectEntityResource = true;
                    }
                    else
                    {
                        if (parentPath.ends_with("/"))
                        {
                            pathPart = pathPart.substr(0, pathPart.find('/'));
                        }
                        else
                        {
                            continue; //Case when pathPart contains part of parent's folder name
                        }
                    }
                }
            }

            if (!isFolder)
            {
                if (!isAspectEntityResource)
                {
                    pathPart = pathPart.substr(0, pathPart.find("]"));
                }

                pathPart += " (" + StringUtility::ToUpperCase(resourceInfo.type) + ")";
            }
            else
            {
                if (pathPart.starts_with("aspectdummy"))
                {
                    pathPart = pathPart.substr(0, pathPart.find("]"));
                }
            }

            if (resourceNodes.contains(pathPart))
            {
                continue;
            }

            if (isFolder)
            {
                resourceNode.children.push_back(ResourceNode());
            }
            else
            {
                resourceNode.hash = resourceInfo.hash;
            }

            resourceNode.name = pathPart;
            resourceNode.index = nodeIndex++;

            resourceNodes[pathPart] = resourceNode;
        }
    }

    parentNode.children.reserve(resourceNodes.size());

    for (auto it = resourceNodes.begin(); it != resourceNodes.end(); ++it)
    {
        parentNode.children.push_back(it->second);
    }
}

void ResourceBrowserPanel::AddRootResourceNodes()
{
    nodeIndex = 0;

    assemblyNode.index = nodeIndex++;
    assemblyNode.hash = -1;
    assemblyNode.name = "assembly:/";
    modulesNode.index = nodeIndex++;
    modulesNode.hash = -1;
    modulesNode.name = "modules:/";

    assemblyNode.children.push_back(ResourceNode());
    modulesNode.children.push_back(ResourceNode());
}

void ResourceBrowserPanel::LoadResourceTypes()
{
    std::ifstream inputFile = std::ifstream("assets/ResourceTypes.txt");

    if (inputFile.is_open())
    {
        std::string line;

        while (getline(inputFile, line))
        {
            resourceTypes.insert(std::make_pair(line, true));
        }

        inputFile.close();
    }
    else
    {
        Logger::GetInstance().Log(Logger::Level::Error, "Failed to open ResourceTypes.txt file!");
    }
}

void ResourceBrowserPanel::LoadResource(std::shared_ptr<Resource> resource, const ResourceNode& resourceNode, const bool loadBackReferences)
{
    const ResourceInfoRegistry::ResourceInfo& resourceInfo = ResourceInfoRegistry::GetInstance().GetResourceInfo(resourceNode.hash);

    if (resourceInfo.headerLibraries.size() > 0)
    {
        resource->LoadResource(0, resourceInfo.headerLibraries[0].chunkIndex, resourceInfo.headerLibraries[0].indexInLibrary, true, true, loadBackReferences);
    }
    else
    {
        resource->LoadResource(0, -1, -1, true, true, loadBackReferences);
    }
}

void ResourceBrowserPanel::CreateResourceDocument(const ResourceNode& resourceNode)
{
    const ImGuiID defaultDockID = Editor::GetInstance().GetLastActiveDocument() ? Editor::GetInstance().GetLastActiveDocument()->GetCurrentDockID() : 0;
    const ResourceInfoRegistry::ResourceInfo& resourceInfo = ResourceInfoRegistry::GetInstance().GetResourceInfo(resourceNode.hash);
    std::string resourceName = ResourceUtility::GetResourceName(resourceInfo.resourceID);

    std::shared_ptr<Resource> resource;
    std::shared_ptr<Document> resourceDocument;

    if (resourceInfo.type == "CPPT")
    {
        std::shared_ptr<CppEntityDocument> templateEntityDocument = std::make_shared<CppEntityDocument>(resourceName.c_str(), ICON_MDI_LANGUAGE_CPP, Document::Type::CppEntity, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(templateEntityDocument->GetCppEntity());
        resourceDocument = std::static_pointer_cast<Document>(templateEntityDocument);
    }
    else if (resourceInfo.type == "CBLU")
    {
        std::shared_ptr<CppEntityBlueprintDocument> cppEntityBlueprintDocument = std::make_shared<CppEntityBlueprintDocument>(resourceName.c_str(), ICON_MDI_LANGUAGE_CPP, Document::Type::CppEntityBlueprint, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(cppEntityBlueprintDocument->GetCppEntityBlueprint());
        resourceDocument = std::static_pointer_cast<Document>(cppEntityBlueprintDocument);
    }
    else if (resourceInfo.type == "TEMP")
    {
        std::shared_ptr<TemplateEntityDocument> templateEntityDocument = std::make_shared<TemplateEntityDocument>(resourceName.c_str(), ICON_MDI_FILE_DOCUMENT, Document::Type::TemplateEntity, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(templateEntityDocument->GetTemplateEntity());
        resourceDocument = std::static_pointer_cast<Document>(templateEntityDocument);
    }
    else if (resourceInfo.type == "TEXT")
    {
        std::shared_ptr<TextureDocument> textureDocument = std::make_shared<TextureDocument>(resourceName.c_str(), ICON_MDI_IMAGE, Document::Type::Texture, resourceInfo.hash, true, defaultDockID);

        resource = std::static_pointer_cast<Resource>(textureDocument->GetTexture());
        resourceDocument = std::static_pointer_cast<Document>(textureDocument);
    }
    else if (resourceInfo.type == "TELI")
    {
        std::shared_ptr<TextListDocument> textureDocument = std::make_shared<TextListDocument>(resourceName.c_str(), ICON_MDI_TEXT_BOX, Document::Type::TextList, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(textureDocument->GetTextList());
        resourceDocument = std::static_pointer_cast<Document>(textureDocument);
    }
    else if (resourceInfo.type == "LOCR")
    {
        std::shared_ptr<LocalizationDocument> localizationDocument = std::make_shared<LocalizationDocument>(resourceName.c_str(), ICON_MDI_TRANSLATE, Document::Type::Localization, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(localizationDocument->GetLocalization());
        resourceDocument = std::static_pointer_cast<Document>(localizationDocument);
    }
    else if (resourceInfo.type == "LOCM")
    {
        std::shared_ptr<MultiLanguageDocument> multiLanguageDocument = std::make_shared<MultiLanguageDocument>(resourceName.c_str(), ICON_MDI_TRANSLATE, Document::Type::MultiLanguage, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(multiLanguageDocument->GetMultiLanguage());
        resourceDocument = std::static_pointer_cast<Document>(multiLanguageDocument);
    }
    else if (resourceInfo.type == "MATT")
    {
        std::shared_ptr<RenderMaterialEntityTypeDocument> renderMaterialEntityTypeDocument = std::make_shared<RenderMaterialEntityTypeDocument>(resourceName.c_str(), ICON_MDI_PALETTE, Document::Type::RenderMaterialEntityType, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(renderMaterialEntityTypeDocument->GetRenderMaterialEntityType());
        resourceDocument = std::static_pointer_cast<Document>(renderMaterialEntityTypeDocument);
    }
    else if (resourceInfo.type == "MATI")
    {
        std::shared_ptr<RenderMaterialInstanceDocument> renderMaterialInstanceDocument = std::make_shared<RenderMaterialInstanceDocument>(resourceName.c_str(), ICON_MDI_PALETTE, Document::Type::RenderMaterialInstance, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(renderMaterialInstanceDocument->GetRenderMaterialInstance());
        resourceDocument = std::static_pointer_cast<Document>(renderMaterialInstanceDocument);
    }
    else if (resourceInfo.type == "MATE")
    {
        std::shared_ptr<RenderMaterialEffectDocument> renderMaterialEffectDocument = std::make_shared<RenderMaterialEffectDocument>(resourceName.c_str(), ICON_MDI_PALETTE, Document::Type::RenderMaterialEffect, resourceInfo.hash, true, defaultDockID);

        resource = std::static_pointer_cast<Resource>(renderMaterialEffectDocument->GetRenderMaterialEffect());
        resourceDocument = std::static_pointer_cast<Document>(renderMaterialEffectDocument);
    }
    else if (resourceInfo.type == "SBPD")
    {
        std::shared_ptr<SoundBlendContainerExternalParametersTypeDocument> soundBlendContainerExternalParametersTypeDocument = std::make_shared<SoundBlendContainerExternalParametersTypeDocument>(resourceName.c_str(), ICON_MDI_VOLUME_HIGH, Document::Type::SoundBlendContainerExternalParametersType, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(soundBlendContainerExternalParametersTypeDocument->GetSoundBlendContainerExternalParametersType());
        resourceDocument = std::static_pointer_cast<Document>(soundBlendContainerExternalParametersTypeDocument);
    }
    else if (resourceInfo.type == "SBPB")
    {
        std::shared_ptr<SoundBlendContainerExternalParametersBlueprintDocument> soundBlendContainerExternalParametersBlueprintDocument = std::make_shared<SoundBlendContainerExternalParametersBlueprintDocument>(resourceName.c_str(), ICON_MDI_VOLUME_HIGH, Document::Type::SoundBlendContainerExternalParametersBlueprint, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(soundBlendContainerExternalParametersBlueprintDocument->GetSoundBlendContainerExternalParametersBlueprint());
        resourceDocument = std::static_pointer_cast<Document>(soundBlendContainerExternalParametersBlueprintDocument);
    }
    else if (resourceInfo.type == "FSBF")
    {
        std::shared_ptr<WaveBankFSBFDocument> waveBankFSBFDocument = std::make_shared<WaveBankFSBFDocument>(resourceName.c_str(), ICON_MDI_VOLUME_HIGH, Document::Type::WaveBankFSBF, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(waveBankFSBFDocument->GetWaveBankFSBF());
        resourceDocument = std::static_pointer_cast<Document>(waveBankFSBFDocument);
    }
    else if (resourceInfo.type == "FSBM")
    {
        std::shared_ptr<WaveBankFSBMDocument> waveBankFSBMDocument = std::make_shared<WaveBankFSBMDocument>(resourceName.c_str(), ICON_MDI_VOLUME_HIGH, Document::Type::WaveBankFSBM, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(waveBankFSBMDocument->GetWaveBankFSBM());
        resourceDocument = std::static_pointer_cast<Document>(waveBankFSBMDocument);
    }
    else if (resourceInfo.type == "FSBS")
    {
        std::shared_ptr<WaveBankFSBSDocument> waveBankFSBSDocument = std::make_shared<WaveBankFSBSDocument>(resourceName.c_str(), ICON_MDI_VOLUME_HIGH, Document::Type::WaveBankFSBS, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(waveBankFSBSDocument->GetWaveBankFSBS());
        resourceDocument = std::static_pointer_cast<Document>(waveBankFSBSDocument);
    }
    else if (resourceInfo.type == "WAVB")
    {
        std::shared_ptr<WaveBankDocument> waveBankDocument = std::make_shared<WaveBankDocument>(resourceName.c_str(), ICON_MDI_VOLUME_HIGH, Document::Type::WaveBank, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(waveBankDocument->GetWaveBank());
        resourceDocument = std::static_pointer_cast<Document>(waveBankDocument);
    }
    else if (resourceInfo.type == "WBFX")
    {
        std::shared_ptr<WaveBankFXDocument> waveBankFXDocument = std::make_shared<WaveBankFXDocument>(resourceName.c_str(), ICON_MDI_VOLUME_HIGH, Document::Type::WaveBankFX, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(waveBankFXDocument->GetWaveBankFX());
        resourceDocument = std::static_pointer_cast<Document>(waveBankFXDocument);
    }
    else if (resourceInfo.type == "SWFF")
    {
        std::shared_ptr<FlashMovieDocument> flashMovieDocument = std::make_shared<FlashMovieDocument>(resourceName.c_str(), ICON_MDI_IMAGE, Document::Type::FlashMovie, resourceInfo.hash, true, defaultDockID);

        resource = std::static_pointer_cast<Resource>(flashMovieDocument->GetFlashMovie());
        resourceDocument = std::static_pointer_cast<Document>(flashMovieDocument);
    }
    else if (resourceInfo.type == "GFXF")
    {
        std::shared_ptr<GFXMovieDocument> gfxMovieDocument = std::make_shared<GFXMovieDocument>(resourceName.c_str(), ICON_MDI_IMAGE, Document::Type::GFXMovie, resourceInfo.hash, true, defaultDockID);

        resource = std::static_pointer_cast<Resource>(gfxMovieDocument->GetGFXMovie());
        resourceDocument = std::static_pointer_cast<Document>(gfxMovieDocument);
    }
    else if (resourceInfo.type == "ChrT")
    {
        std::shared_ptr<AnimationDatabaseDocument> animationDatabaseDocument = std::make_shared<AnimationDatabaseDocument>(resourceName.c_str(), ICON_MDI_DATABASE, Document::Type::AnimationDatabase, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(animationDatabaseDocument->GetAnimationDatabase());
        resourceDocument = std::static_pointer_cast<Document>(animationDatabaseDocument);
    }
    else if (resourceInfo.type == "SDEF")
    {
        std::shared_ptr<SoundDefinitionsDocument> soundDefinitionsDocument = std::make_shared<SoundDefinitionsDocument>(resourceName.c_str(), ICON_MDI_VOLUME_HIGH, Document::Type::SoundDefinitions, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(soundDefinitionsDocument->GetSoundDefinitions());
        resourceDocument = std::static_pointer_cast<Document>(soundDefinitionsDocument);
    }
    else if (resourceInfo.type == "GIDX")
    {
        std::shared_ptr<GlobalResourceIndexDocument> globalResourceIndexDocument = std::make_shared<GlobalResourceIndexDocument>(resourceName.c_str(), ICON_MDI_VIEW_LIST, Document::Type::SoundDefinitions, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(globalResourceIndexDocument->GetGlobalResourceIndex());
        resourceDocument = std::static_pointer_cast<Document>(globalResourceIndexDocument);
    }
    else if (resourceInfo.type == "AIBB")
    {
        std::shared_ptr<BehaviorTreeEntityBlueprintDocument> behaviorTreeEntityBlueprintDocument = std::make_shared<BehaviorTreeEntityBlueprintDocument>(resourceName.c_str(), ICON_MDI_VIEW_LIST, Document::Type::SoundDefinitions, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(behaviorTreeEntityBlueprintDocument->GetBehaviorTreeEntityBlueprint());
        resourceDocument = std::static_pointer_cast<Document>(behaviorTreeEntityBlueprintDocument);
    }
    else if (resourceInfo.type == "MUCB")
    {
        std::shared_ptr<CompositionBlueprintDocument> compositionBlueprintDocument = std::make_shared<CompositionBlueprintDocument>(resourceName.c_str(), ICON_MDI_VOLUME_HIGH, Document::Type::SoundDefinitions, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(compositionBlueprintDocument->GetCompositionBlueprint());
        resourceDocument = std::static_pointer_cast<Document>(compositionBlueprintDocument);
    }
    else if (resourceInfo.type == "PKGL")
    {
        std::shared_ptr<PackageListDocument> packageListDocument = std::make_shared<PackageListDocument>(resourceName.c_str(), ICON_MDI_VIEW_LIST, Document::Type::SoundDefinitions, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(packageListDocument->GetPackageList());
        resourceDocument = std::static_pointer_cast<Document>(packageListDocument);
    }
    else if (resourceInfo.type == "SCDA")
    {
        std::shared_ptr<ScatterDataDocument> scatterDataDocument = std::make_shared<ScatterDataDocument>(resourceName.c_str(), ICON_MDI_GRASS, Document::Type::SoundDefinitions, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(scatterDataDocument->GetScatterData());
        resourceDocument = std::static_pointer_cast<Document>(scatterDataDocument);
    }
    else if (resourceInfo.type == "CLOT")
    {
        std::shared_ptr<FabricResourceEntityDocument> fabricResourceEntityDocument = std::make_shared<FabricResourceEntityDocument>(resourceName.c_str(), ICON_MDI_TSHIRT_CREW, Document::Type::SoundDefinitions, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(fabricResourceEntityDocument->GetFabricResourceEntity());
        resourceDocument = std::static_pointer_cast<Document>(fabricResourceEntityDocument);
    }
    else if (resourceInfo.type == "PRIM")
    {
        std::shared_ptr<RenderPrimitiveDocument> renderPrimitiveDocument = std::make_shared<RenderPrimitiveDocument>(resourceName.c_str(), ICON_MDI_SHAPE, Document::Type::SoundDefinitions, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(renderPrimitiveDocument->GetRenderPrimitive());
        resourceDocument = std::static_pointer_cast<Document>(renderPrimitiveDocument);
    }
    else if (resourceInfo.type == "CLOS")
    {
        std::shared_ptr<ClothDocument> clothDocument = std::make_shared<ClothDocument>(resourceName.c_str(), ICON_MDI_SHAPE, Document::Type::SoundDefinitions, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(clothDocument->GetCloth());
        resourceDocument = std::static_pointer_cast<Document>(clothDocument);
    }
    else if (resourceInfo.type == "BORG")
    {
        std::shared_ptr<BoneRigDocument> boneRigDocument = std::make_shared<BoneRigDocument>(resourceName.c_str(), ICON_MDI_SHAPE, Document::Type::SoundDefinitions, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(boneRigDocument->GetBoneRig());
        resourceDocument = std::static_pointer_cast<Document>(boneRigDocument);
    }
    else if (resourceInfo.type == "ALOC")
    {
        std::shared_ptr<PhysicsDocument> physicsDocument = std::make_shared<PhysicsDocument>(resourceName.c_str(), ICON_MDI_SHAPE, Document::Type::SoundDefinitions, resourceInfo.hash, false, defaultDockID);

        resource = std::static_pointer_cast<Resource>(physicsDocument->GetPhysics());
        resourceDocument = std::static_pointer_cast<Document>(physicsDocument);
    }

    if (!resource || !resourceDocument)
    {
        Logger::GetInstance().Log(Logger::Level::Warning, "Cannot open resource: unsupported type.");
        return;
    }

    resource->SetHash(resourceInfo.hash);
    resource->SetResourceID(resourceInfo.resourceID);
    resource->SetHeaderLibraries(&resourceInfo.headerLibraries);
    resource->SetName(resourceName);

    Editor::GetInstance().GetDocuments().push_back(resourceDocument);
    std::thread thread(&ResourceBrowserPanel::LoadResource, this, resource, resourceNode, true);

    thread.detach();
}

void ResourceBrowserPanel::RenderFolderContextMenu(ResourceNode& folderNode, const std::string& parentPath)
{
    static std::string batchExportLabel = std::format("{} Batch Export Localizations", ICON_MDI_EXPORT);

    if (ImGui::MenuItem(batchExportLabel.c_str()))
    {
        showBatchExportPopup = true;
        batchExportFolderNode = folderNode;
        batchExportFolderPath = parentPath;
        if (batchExportFolderPath.ends_with("/"))
        {
            batchExportFolderPath = batchExportFolderPath.substr(0, batchExportFolderPath.length() - 1);
        }

        if (batchExportLanguages.empty())
        {
            batchExportLanguages["_en"] = true;
            batchExportLanguages["_de"] = false;
            batchExportLanguages["_fr"] = false;
            batchExportLanguages["_it"] = false;
            batchExportLanguages["_es"] = false;
            batchExportLanguages["_pl"] = true;
            batchExportLanguages["_ru"] = false;
            batchExportLanguages["_tr"] = false;
            batchExportLanguages["_ja"] = false;
        }
    }

    static std::string batchImportLabel = std::format("{} Batch Import Localizations", ICON_MDI_IMPORT);

    if (ImGui::MenuItem(batchImportLabel.c_str()))
    {
        showBatchImportPopup = true;
        batchImportInputFolder = "";
    }
}

void ResourceBrowserPanel::RenderBatchExportPopup()
{
    if (showBatchExportPopup)
    {
        ImGui::OpenPopup("Batch Export Localizations");
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 modalSize = ImVec2(500, 400);
    ImVec2 centerPosition = ImVec2(
        viewport->GetCenter().x - modalSize.x / 2,
        viewport->GetCenter().y - modalSize.y / 2
    );

    ImGui::SetNextWindowSize(modalSize);
    ImGui::SetNextWindowPos(centerPosition, ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Batch Export Localizations", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::PushFont(Editor::GetInstance().GetImGuiRenderer()->GetMiddleFont());

        ImGui::TextUnformatted("Output Folder Path");
        const float windowWidth = ImGui::GetContentRegionAvail().x;
        const float buttonWidth = UI::GetIconButtonSize(ICON_MDI_FOLDER, "").x;
        const float inputTextWidth = windowWidth - buttonWidth - ImGui::GetStyle().ItemSpacing.x;

        ImGui::PushItemWidth(inputTextWidth);
        ImGui::InputText("##OutputFolderPath", &batchExportOutputFolder);
        ImGui::PopItemWidth();
        ImGui::SameLine();

        if (ImGui::Button(ICON_MDI_FOLDER))
        {
            batchExportOutputFolder = FileDialog::OpenFolder();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Select languages to export:");
        ImGui::Spacing();

        if (ImGui::Button("Select All"))
        {
            for (auto& pair : batchExportLanguages) pair.second = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Deselect All"))
        {
            for (auto& pair : batchExportLanguages) pair.second = false;
        }
        ImGui::Spacing();

        for (auto& pair : batchExportLanguages)
        {
            ImGui::Checkbox(pair.first.c_str(), &pair.second);
        }

        ImGui::Spacing();
        ImGui::Spacing();

        if (UI::IconButton("  " ICON_MDI_EXPORT, " Export "))
        {
            showBatchExportPopup = false;
            ImGui::CloseCurrentPopup();

            if (!batchExportOutputFolder.empty())
            {
                std::vector<std::string> activeLanguages;
                for (auto& pair : batchExportLanguages)
                {
                    if (pair.second)
                    {
                        activeLanguages.push_back(pair.first);
                    }
                }

                TaskRunner::RunAsync([this, batchExportFolderNode = this->batchExportFolderNode, batchExportFolderPath = this->batchExportFolderPath, batchExportOutputFolder = this->batchExportOutputFolder, activeLanguages]() {
                    ExecuteBatchExport(batchExportFolderNode, batchExportFolderPath, batchExportOutputFolder, activeLanguages);
                });
            }
        }

        ImGui::SameLine();

        if (UI::IconButton("  " ICON_MDI_CLOSE, " Cancel "))
        {
            showBatchExportPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::PopFont();
        ImGui::EndPopup();
    }
}

void ResourceBrowserPanel::ExecuteBatchExport(ResourceNode folderNode, std::string basePath, std::string outputPath, std::vector<std::string> languages)
{
    Logger::GetInstance().Log(Logger::Level::Info, std::format("Starting batch export of localization files from '{}'", basePath));

    std::vector<std::pair<ResourceNode, std::string>> nodesToProcess;
    std::vector<std::pair<ResourceNode, std::string>> allLeafNodes;
    nodesToProcess.push_back({ folderNode, "" });

    while (!nodesToProcess.empty())
    {
        auto [currentNode, currentRelPath] = nodesToProcess.back();
        nodesToProcess.pop_back();

        if (currentNode.children.size() == 1 && currentNode.children[0].name.empty())
        {
            std::string fullPath = basePath;
            if (!currentRelPath.empty()) fullPath += "/" + currentRelPath;
            AddChildren(currentNode, fullPath);
        }

        for (ResourceNode& child : currentNode.children)
        {
            std::string childRelPath = currentRelPath.empty() ? child.name : currentRelPath + "/" + child.name;

            if (child.children.size() > 0) 
            {
                nodesToProcess.push_back({ child, childRelPath });
            }
            else
            {
                allLeafNodes.push_back({ child, currentRelPath });
            }
        }
    }

    int exportedCount = 0;

    for (const auto& [leafNode, relPath] : allLeafNodes)
    {
        const ResourceInfoRegistry::ResourceInfo& resInfo = ResourceInfoRegistry::GetInstance().GetResourceInfo(leafNode.hash);
        
        if (resInfo.type == "TELI" || resInfo.type == "LOCR")
        {
            bool matchesLanguage = false;
            std::string leafNameLower = StringUtility::ToLowerCase(leafNode.name);

            if (languages.empty()) 
            {
                matchesLanguage = true; 
            }
            else
            {
                for (const std::string& lang : languages)
                {
                    if (leafNameLower.find(lang + ".") != std::string::npos)
                    {
                        matchesLanguage = true;
                        break;
                    }
                }
            }

            if (matchesLanguage)
            {
                try
                {
                    std::shared_ptr<Resource> resourceToExport = ResourceUtility::CreateResource(resInfo.type);
                    std::string resourceName = ResourceUtility::GetResourceName(resInfo.resourceID);

                    resourceToExport->SetHash(resInfo.hash);
                    resourceToExport->SetResourceID(resInfo.resourceID);
                    resourceToExport->SetHeaderLibraries(&resInfo.headerLibraries);
                    resourceToExport->SetName(resourceName);

                    ResourceUtility::LoadResource(resourceToExport);

                    if (resourceToExport->GetResourceData())
                    {
                        resourceToExport->Deserialize();

                        std::string fullOutDir = outputPath;
                        if (!relPath.empty())
                        {
                            std::string fixedRelPath = relPath;
                            std::replace(fixedRelPath.begin(), fixedRelPath.end(), '/', '\\');
                            fullOutDir += "\\" + fixedRelPath;
                        }

                        std::filesystem::create_directories(fullOutDir);

                        std::string outFilePath = fullOutDir + "\\" + ResourceUtility::GenerateFileName(resourceToExport) + ".TEXTLIST.JSON";
                        resourceToExport->Export(outFilePath, "Json File (.TEXTLIST.JSON)");
                        
                        exportedCount++;
                    }
                    else
                    {
                        Logger::GetInstance().Log(Logger::Level::Warning, std::format("Skipped exporting {} because it has no data.", resourceName));
                    }
                }
                catch (const std::exception& e)
                {
                    Logger::GetInstance().Log(Logger::Level::Error, std::format("Exception during batch export of {}: {}", leafNode.name, e.what()));
                }
            }
        }
    }

    Logger::GetInstance().Log(Logger::Level::Info, std::format("Batch export complete. Exported {} files.", exportedCount));
}

void ResourceBrowserPanel::RenderBatchImportPopup()
{
    if (showBatchImportPopup)
    {
        ImGui::OpenPopup("Batch Import Localizations");
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 modalSize = ImVec2(500, 200);
    ImVec2 centerPosition = ImVec2(
        viewport->GetCenter().x - modalSize.x / 2,
        viewport->GetCenter().y - modalSize.y / 2
    );

    ImGui::SetNextWindowSize(modalSize);
    ImGui::SetNextWindowPos(centerPosition, ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Batch Import Localizations", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::PushFont(Editor::GetInstance().GetImGuiRenderer()->GetMiddleFont());

        ImGui::TextUnformatted("Input Folder Path (JSON Files)");
        const float windowWidth = ImGui::GetContentRegionAvail().x;
        const float buttonWidth = UI::GetIconButtonSize(ICON_MDI_FOLDER, "").x;
        const float inputTextWidth = windowWidth - buttonWidth - ImGui::GetStyle().ItemSpacing.x;

        ImGui::PushItemWidth(inputTextWidth);
        ImGui::InputText("##InputFolderPath", &batchImportInputFolder);
        ImGui::PopItemWidth();
        ImGui::SameLine();

        if (ImGui::Button(ICON_MDI_FOLDER))
        {
            batchImportInputFolder = FileDialog::OpenFolder();
        }

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();

        if (UI::IconButton("  " ICON_MDI_IMPORT, " Import "))
        {
            showBatchImportPopup = false;
            ImGui::CloseCurrentPopup();

            if (!batchImportInputFolder.empty())
            {
                TaskRunner::RunAsync([this, batchImportInputFolder = this->batchImportInputFolder]() {
                    ExecuteBatchImport(batchImportInputFolder);
                });
            }
        }

        ImGui::SameLine();

        if (UI::IconButton("  " ICON_MDI_CLOSE, " Cancel "))
        {
            showBatchImportPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::PopFont();
        ImGui::EndPopup();
    }
}

void ResourceBrowserPanel::ExecuteBatchImport(std::string inputPath)
{
    Logger::GetInstance().Log(Logger::Level::Info, std::format("Starting batch import from '{}'", inputPath));

    if (!std::filesystem::exists(inputPath) || !std::filesystem::is_directory(inputPath))
    {
        Logger::GetInstance().Log(Logger::Level::Error, "Invalid input folder for batch import.");
        return;
    }

    int importedCount = 0;
    int failedCount = 0;

    const auto& resourcesInfo = ResourceInfoRegistry::GetInstance().GetResourcesInfo();
    std::unordered_map<std::string, unsigned long long> resourceIDToHash;
    for (const auto& [hash, info] : resourcesInfo)
    {
        resourceIDToHash[info.resourceID] = hash;
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(inputPath))
    {
        if (entry.is_regular_file())
        {
            std::string filePath = entry.path().string();
            std::string fileName = entry.path().filename().string();
            std::string fileNameLower = StringUtility::ToLowerCase(fileName);

            if (fileNameLower.ends_with(".json"))
            {
                // Format: resourceName_0xHASH.TEXTLIST.JSON
                size_t lastUnderscore = fileName.find_last_of('_');
                size_t firstDot = fileName.find('.', lastUnderscore);
                
                if (lastUnderscore != std::string::npos && firstDot != std::string::npos && firstDot > lastUnderscore)
                {
                    std::string hexStr = fileName.substr(lastUnderscore + 1, firstDot - lastUnderscore - 1);
                    if (hexStr.starts_with("0x") || hexStr.starts_with("0X"))
                    {
                        try 
                        {
                            unsigned long long runtimeResourceID = std::stoull(hexStr, nullptr, 16);
                            std::string resourceID = ResourceIDRegistry::GetInstance().GetResourceID(runtimeResourceID);

                            if (!resourceID.empty() && resourceIDToHash.contains(resourceID))
                            {
                                unsigned long long hash = resourceIDToHash[resourceID];
                                const ResourceInfoRegistry::ResourceInfo& resInfo = ResourceInfoRegistry::GetInstance().GetResourceInfo(hash);
                                std::shared_ptr<Resource> importResource = ResourceUtility::CreateResource(resInfo.type);
                                std::string resName = ResourceUtility::GetResourceName(resInfo.resourceID);
                                
                                importResource->SetHash(resInfo.hash);
                                importResource->SetResourceID(resInfo.resourceID);
                                importResource->SetHeaderLibraries(&resInfo.headerLibraries);
                                importResource->SetName(resName);

                                // Load the resource header to find offset and sizes
                                importResource->LoadResource(0, resInfo.headerLibraries[0].chunkIndex, resInfo.headerLibraries[0].indexInLibrary, true, false, true);
                                
                                if (resInfo.type == "TELI")
                                {
                                    std::shared_ptr<TextList> textList = std::static_pointer_cast<TextList>(importResource);
                                    
                                    bool importSuccess = false;
                                    try 
                                    {
                                        textList->ImportFromJson(filePath);
                                        importSuccess = true;
                                    }
                                    catch (const std::exception& e)
                                    {
                                        Logger::GetInstance().Log(Logger::Level::Error, "Failed to parse JSON for file: {}. Error: {}", fileName, e.what());
                                    }

                                    if (importSuccess)
                                    {
                                        std::string resourceLibPath = importResource->GetResourceLibraryFilePath();
                                        std::string headerLibPath = importResource->GetHeaderLibraryFilePath();
                                        unsigned int offsetInResLib = importResource->GetOffsetInResourceLibrary();
                                        unsigned int offsetInHeaderLib = importResource->GetOffsetInHeaderLibrary();
                                        const void* newData = importResource->GetResourceData();
                                        unsigned int newDataSize = importResource->GetResourceDataSize();

                                        if (ResourcePatcher::PatchResourceLibrary(resourceLibPath, headerLibPath, offsetInResLib, offsetInHeaderLib, newData, newDataSize))
                                        {
                                            importedCount++;
                                        }
                                        else
                                        {
                                            Logger::GetInstance().Log(Logger::Level::Error, "Failed to patch resource for file: {}", fileName);
                                            failedCount++;
                                        }
                                    }
                                    else
                                    {
                                        failedCount++;
                                    }
                                }
                                else if (resInfo.type == "LOCR")
                                {
                                    std::shared_ptr<Localization> localization = std::static_pointer_cast<Localization>(importResource);
                                    
                                    bool importSuccess = false;
                                    try 
                                    {
                                        localization->ImportFromJson(filePath);
                                        importSuccess = true;
                                    }
                                    catch (const std::exception& e)
                                    {
                                        Logger::GetInstance().Log(Logger::Level::Error, "Failed to parse JSON for file: {}. Error: {}", fileName, e.what());
                                    }
                                    
                                    if (importSuccess)
                                    {
                                        std::string resourceLibPath = importResource->GetResourceLibraryFilePath();
                                        std::string headerLibPath = importResource->GetHeaderLibraryFilePath();
                                        unsigned int offsetInResLib = importResource->GetOffsetInResourceLibrary();
                                        unsigned int offsetInHeaderLib = importResource->GetOffsetInHeaderLibrary();
                                        const void* newData = importResource->GetResourceData();
                                        unsigned int newDataSize = importResource->GetResourceDataSize();

                                        if (ResourcePatcher::PatchResourceLibrary(resourceLibPath, headerLibPath, offsetInResLib, offsetInHeaderLib, newData, newDataSize))
                                        {
                                            importedCount++;
                                        }
                                        else
                                        {
                                            Logger::GetInstance().Log(Logger::Level::Error, "Failed to patch resource for file: {}", fileName);
                                            failedCount++;
                                        }
                                    }
                                    else
                                    {
                                        failedCount++;
                                    }
                                }
                                else if (resInfo.type == "LOCM")
                                {
                                    std::shared_ptr<MultiLanguage> multiLanguage = std::static_pointer_cast<MultiLanguage>(importResource);
                                    
                                    bool importSuccess = false;
                                    try 
                                    {
                                        multiLanguage->ImportFromJson(filePath);
                                        importSuccess = true;
                                    }
                                    catch (const std::exception& e)
                                    {
                                        Logger::GetInstance().Log(Logger::Level::Error, "Failed to parse JSON for file: {}. Error: {}", fileName, e.what());
                                    }
                                    
                                    if (importSuccess)
                                    {
                                        std::string resourceLibPath = importResource->GetResourceLibraryFilePath();
                                        std::string headerLibPath = importResource->GetHeaderLibraryFilePath();
                                        unsigned int offsetInResLib = importResource->GetOffsetInResourceLibrary();
                                        unsigned int offsetInHeaderLib = importResource->GetOffsetInHeaderLibrary();
                                        const void* newData = importResource->GetResourceData();
                                        unsigned int newDataSize = importResource->GetResourceDataSize();

                                        if (ResourcePatcher::PatchResourceLibrary(resourceLibPath, headerLibPath, offsetInResLib, offsetInHeaderLib, newData, newDataSize))
                                        {
                                            importedCount++;
                                        }
                                        else
                                        {
                                            Logger::GetInstance().Log(Logger::Level::Error, "Failed to patch resource for file: {}", fileName);
                                            failedCount++;
                                        }
                                    }
                                    else
                                    {
                                        failedCount++;
                                    }
                                }
                                else 
                                {
                                    Logger::GetInstance().Log(Logger::Level::Warning, "Unsupported resource type for batch import: {}", fileName);
                                    failedCount++;
                                }
                            }
                            else
                            {
                                Logger::GetInstance().Log(Logger::Level::Warning, "Resource ID not found in registry for file: {}", fileName);
                                failedCount++;
                            }
                        }
                        catch (const std::exception& e)
                        {
                            Logger::GetInstance().Log(Logger::Level::Error, "Failed to process file {}: {}", fileName, e.what());
                            failedCount++;
                        }
                    }
                }
            }
        }
    }

    Logger::GetInstance().Log(Logger::Level::Info, std::format("Batch import complete. Successfully imported {}, Failed {}", importedCount, failedCount));
}
