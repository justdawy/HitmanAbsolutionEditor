#include <fstream>
#include <string_view>

#include <IconsMaterialDesignIcons.h>
#include "misc/cpp/imgui_stdlib.h"

#include "UI/Panels/DeepSearchPanel.h"
#include "UI/Panels/ResourceBrowserPanel.h"
#include "Registry/ResourceInfoRegistry.h"
#include "Utility/ResourceUtility.h"
#include "Utility/StringUtility.h"
#include "Utility/UI.h"
#include "Logger.h"
#include "Editor.h"

DeepSearchPanel::DeepSearchPanel(const char* name, const char* icon) : BasePanel(name, icon)
{
	LoadResourceTypes();
}

DeepSearchPanel::~DeepSearchPanel()
{
	StopSearch();
}

void DeepSearchPanel::LoadResourceTypes()
{
	std::ifstream inputFile("assets/ResourceTypes.txt");

	if (inputFile.is_open())
	{
		std::string line;

		while (getline(inputFile, line))
		{
			// By default, check only common text/UI types to save time, others false
			bool checkByDefault = (line == "TELI" || line == "LOCR" || line == "LOCM" || line == "SWFF" || line == "GFXF");
			resourceTypes.insert(std::make_pair(line, checkByDefault));
		}

		inputFile.close();
	}
	else
	{
		Logger::GetInstance().Log(Logger::Level::Error, "Failed to open ResourceTypes.txt file!");
	}
}

void DeepSearchPanel::StopSearch()
{
	stopSearchRequested = true;
	if (searchThread.joinable())
	{
		searchThread.join();
	}
	isSearching = false;
}

void DeepSearchPanel::PerformDeepSearch()
{
	isSearching = true;
	stopSearchRequested = false;
	resourcesSearched = 0;

	std::string searchStr = searchText;
	if (searchStr.empty())
	{
		isSearching = false;
		return;
	}

	std::vector<std::string> activeTypes;
	for (auto& [type, active] : resourceTypes)
	{
		if (active) activeTypes.push_back(type);
	}

	const auto& allResources = ResourceInfoRegistry::GetInstance().GetResourcesInfo();
	std::vector<std::pair<unsigned long long, ResourceInfoRegistry::ResourceInfo>> filteredResources;
	
	for (const auto& [hash, info] : allResources)
	{
		if (std::find(activeTypes.begin(), activeTypes.end(), info.type) != activeTypes.end())
		{
			filteredResources.push_back({hash, info});
		}
	}

	totalResourcesToSearch = filteredResources.size();
	unsigned int nodeIndex = 0;

	for (const auto& [hash, info] : filteredResources)
	{
		if (stopSearchRequested) break;

		try
		{
			std::shared_ptr<Resource> resource = ResourceUtility::CreateResource(info.type);
			if (resource)
			{
				resource->SetHash(hash);
				resource->SetResourceID(info.resourceID);
				resource->SetHeaderLibraries(&info.headerLibraries);
				
				if (info.headerLibraries.size() > 0)
				{
					resource->LoadResource(0, info.headerLibraries[0].chunkIndex, info.headerLibraries[0].indexInLibrary, false, false, true);
				}
				else
				{
					resource->LoadResource(0, -1, -1, false, false, true);
				}

				const void* data = resource->GetResourceData();
				unsigned int size = resource->GetResourceDataSize();

				if (data && size > 0)
				{
					std::string_view sv(static_cast<const char*>(data), size);
					if (sv.find(searchStr) != std::string_view::npos)
					{
						SearchResult result;
						result.index = nodeIndex++;
						result.hash = hash;
						result.hashStr = StringUtility::ConvertValueToHexString(hash);
						result.resourceID = info.resourceID;
						result.type = info.type;

						std::lock_guard<std::mutex> lock(resultsMutex);
						searchResults.push_back(result);
					}
				}
			}
		}
		catch (...)
		{
			// Ignore any allocation errors or reading exceptions for this specific resource to avoid crashing the whole thread
		}
		resourcesSearched++;
	}

	isSearching = false;
}

void DeepSearchPanel::Render()
{
	if (!Begin())
	{
		return;
	}

	ImGui::PushFont(Editor::GetInstance().GetImGuiRenderer()->GetMiddleFont());
	ImGui::Text("Deep Search");
	ImGui::Spacing();

	static ImVec2 childSize = ImVec2(0, 150);
	static std::string hint = std::format("{} Search text inside files...", ICON_MDI_MAGNIFY);

	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::InputTextWithHint("##SearchInput", hint.c_str(), searchText, IM_ARRAYSIZE(searchText));
	ImGui::PopItemWidth();
	ImGui::Spacing();

	ImGui::Text("Resource Types to Search");
	ImGui::Spacing();
	ImGui::BeginChild("Resource Types", childSize, true);

	for (auto it = resourceTypes.begin(); it != resourceTypes.end(); ++it)
	{
		ImGui::Checkbox(it->first.c_str(), &it->second);
	}

	ImGui::EndChild();
	ImGui::Spacing();

	static bool selectAllResourceTypes = false;

	if (ImGui::Checkbox("Select All Resource Types", &selectAllResourceTypes))
	{
		for (auto it = resourceTypes.begin(); it != resourceTypes.end(); ++it)
		{
			it->second = selectAllResourceTypes;
		}
	}

	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::BeginDisabled(isSearching || searchText[0] == '\0');
	if (ImGui::Button("Search"))
	{
		StopSearch();
		
		std::lock_guard<std::mutex> lock(resultsMutex);
		searchResults.clear();
		selectedNodeIndex = -1;
		selectedNodeIndices.clear();

		searchThread = std::thread(&DeepSearchPanel::PerformDeepSearch, this);
	}
	ImGui::EndDisabled();

	if (isSearching)
	{
		ImGui::SameLine();
		if (ImGui::Button("Stop Search"))
		{
			stopSearchRequested = true;
		}
		ImGui::SameLine();
		ImGui::Text("Searching... %d / %d", resourcesSearched.load(), totalResourcesToSearch);
	}
	else if (resourcesSearched > 0 && resourcesSearched == totalResourcesToSearch)
	{
		ImGui::SameLine();
		ImGui::Text("Search completed.");
	}
	else if (resourcesSearched > 0 && stopSearchRequested)
	{
		ImGui::SameLine();
		ImGui::Text("Search stopped.");
	}

	ImGui::Spacing();
	ImGui::Text("Results");
	ImGui::BeginChild("Results Tree", ImVec2(0, 0), true);

	std::lock_guard<std::mutex> lock(resultsMutex);
	for (size_t i = 0; i < searchResults.size(); ++i)
	{
		RenderTree(searchResults[i]);
	}

	ImGui::EndChild();

	if (selectedNodeIndex != -1)
	{
		if (ImGui::GetIO().KeyCtrl)
		{
			selectedNodeIndices.insert(selectedNodeIndex);
			selectedNodeIndex = -1;
		}
		else
		{
			selectedNodeIndices.clear();
			selectedNodeIndices.insert(selectedNodeIndex);
		}
	}

	ImGui::PopFont();
	End();
}

void DeepSearchPanel::RenderTree(SearchResult& result)
{
	ImGui::PushID(result.index);

	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	const bool isNodeSelected = selectedNodeIndices.contains(result.index);

	if (isNodeSelected)
	{
		nodeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	std::string label = std::format("{} {} [{}] ({})", ICON_MDI_FILE, result.resourceID, result.hashStr, result.type);

	ImGui::TreeNodeEx(label.c_str(), nodeFlags);

	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
	{
		selectedNodeIndex = result.index;
	}

	if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
	{
		// Open the resource in a new document, similar to ResourceBrowserPanel
		ResourceBrowserPanel::ResourceNode resourceNode;
		resourceNode.hash = result.hash;
		resourceNode.name = result.resourceID;
		
		bool resourceOpened = false;
		for (size_t i = 0; i < Editor::GetInstance().GetDocuments().size(); ++i)
		{
			auto document = Editor::GetInstance().GetDocuments()[i];
			if (document->GetType() == Document::Type::Scene)
			{
				for (auto& panel : document->GetPanels())
				{
					std::shared_ptr<ResourceBrowserPanel> browser = std::dynamic_pointer_cast<ResourceBrowserPanel>(panel);
					if (browser)
					{
						browser->CreateResourceDocument(resourceNode);
						resourceOpened = true;
						break;
					}
				}
			}
			if (resourceOpened) break;
		}
	}

	ImGui::PopID();
}
