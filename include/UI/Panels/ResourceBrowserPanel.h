#pragma once

#include <vector>
#include <map>
#include <set>
#include <d3d11.h>

#include "BasePanel.h"
#include "UI/Documents/Document.h"
#include "Resources/Resource.h"
#include "Rendering/DirectXRenderer.h"

class ResourceBrowserPanel : public BasePanel
{
public:
	struct ResourceNode
	{
		unsigned int index;
		unsigned long long hash;
		std::string name;
		std::vector<ResourceNode> children;
	};

	ResourceBrowserPanel(const char* name, const char* icon);
	~ResourceBrowserPanel();
	void Render() override;
	void RenderTree(ResourceNode& parentNode, std::string parentPath);
	void RenderContextMenu(ResourceNode& resourceNode);
	void AddChildren(ResourceNode& parentNode, const std::string& parentPath);
	void AddRootResourceNodes();
	void LoadResourceTypes();
	void LoadResource(std::shared_ptr<Resource> resource, const ResourceNode& resourceNode, const bool loadBackReferences = true);
	void CreateResourceDocument(const ResourceNode& resourceNode);
	void RenderFolderContextMenu(ResourceNode& folderNode, const std::string& parentPath);
	void RenderBatchExportPopup();
	void ExecuteBatchExport(ResourceNode folderNode, std::string basePath, std::string outputPath, std::vector<std::string> languages);
	void RenderBatchImportPopup();
	void ExecuteBatchImport(std::string inputPath);

private:
	ResourceNode assemblyNode;
	ResourceNode modulesNode;
	std::map<std::string, bool> resourceTypes;
	char resourceName[256] = { "" };
	bool isInputTextActive;
	unsigned int nodeIndex;
	int selectedNodeIndex;

	bool showResourceExportPopup;
	std::shared_ptr<Resource> resource;

	bool showBatchExportPopup;
	ResourceNode batchExportFolderNode;
	std::string batchExportFolderPath;
	std::string batchExportOutputFolder;
	std::map<std::string, bool> batchExportLanguages;

	bool showBatchImportPopup;
	std::string batchImportInputFolder;
};
