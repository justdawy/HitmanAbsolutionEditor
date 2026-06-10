#pragma once

#include <map>
#include <vector>
#include <set>
#include <string>
#include <atomic>
#include <mutex>
#include <thread>

#include "BasePanel.h"

class DeepSearchPanel : public BasePanel
{
public:
	struct SearchResult
	{
		unsigned int index;
		std::string resourceID;
		std::string hashStr;
		unsigned long long hash;
		std::string type;
	};

	DeepSearchPanel(const char* name, const char* icon);
	~DeepSearchPanel();
	void Render() override;
	void LoadResourceTypes();
	void PerformDeepSearch();
	void RenderTree(SearchResult& result);

private:
	void StopSearch();

	std::map<std::string, bool> resourceTypes;
	char searchText[512]{ "" };
	
	std::vector<SearchResult> searchResults;
	std::mutex resultsMutex;
	
	std::atomic<bool> isSearching{ false };
	std::atomic<bool> stopSearchRequested{ false };
	std::thread searchThread;
	
	int totalResourcesToSearch = 0;
	std::atomic<int> resourcesSearched{ 0 };
	
	unsigned int selectedNodeIndex = -1;
	std::set<unsigned int> selectedNodeIndices;
};
