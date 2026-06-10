#include "UI/Panels/MultiLanguagePanel.h"
#include "Utility/FileDialog.h"
#include "Utility/ResourcePatcher.h"

MultiLanguagePanel::MultiLanguagePanel(const char* name, const char* icon, std::shared_ptr<MultiLanguage> multiLanguageResource) : BasePanel(name, icon)
{
	this->multiLanguageResource = multiLanguageResource;

	tableColumns.push_back({ "Name/Hash" , 0, 1.f });
	tableColumns.push_back({ "Text" , ImGuiTableColumnFlags_WidthStretch, 0.f });
}

void MultiLanguagePanel::Render()
{
	if (!Begin())
	{
		return;
	}

	if (!multiLanguageResource->IsResourceDeserialized())
	{
		ImGui::SetCursorPos(ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y / 2));
		ImGui::Text("Loading multi language resource...");
		End();

		return;
	}

	if (ImGui::Button("Import JSON"))
	{
		std::string filePath = FileDialog::OpenFile("JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0");
		if (!filePath.empty())
		{
			multiLanguageResource->ImportFromJson(filePath);
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Export RAW"))
	{
		std::string filePath = FileDialog::SaveFile("RAW Files (*.raw)\0*.raw\0All Files (*.*)\0*.*\0");
		if (!filePath.empty())
		{
			multiLanguageResource->ExportRawData(filePath);
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Patch Back to Game"))
	{
		multiLanguageResource->SerializeToBuffer(); // Ensure buffer is up to date
		
		std::string resourceLibPath = multiLanguageResource->GetResourceLibraryFilePath();
		std::string headerLibPath = multiLanguageResource->GetHeaderLibraryFilePath();
		unsigned int offsetInResLib = multiLanguageResource->GetOffsetInResourceLibrary();
		unsigned int offsetInHeaderLib = multiLanguageResource->GetOffsetInHeaderLibrary();
		const void* newData = multiLanguageResource->GetResourceData();
		unsigned int newDataSize = multiLanguageResource->GetResourceDataSize();

		if (ResourcePatcher::PatchResourceLibrary(resourceLibPath, headerLibPath, offsetInResLib, offsetInHeaderLib, newData, newDataSize))
		{
			// Success
		}
	}

	ImGui::Separator();

	if (!UI::BeginProperties("MultiLanguage", tableColumns))
	{
		End();

		return;
	}

	char localizationCategory = multiLanguageResource->GetLocalizationCategory();
	std::vector<std::string>& locales = multiLanguageResource->GetLocales();
	std::vector<int>& indices = multiLanguageResource->GetIndices();

	UI::Property("localizationCategory", localizationCategory);

	const bool isLocalesTreeNodeOpen = UI::BeginTreeNodeProperty("locales", nullptr);

	if (isLocalesTreeNodeOpen)
	{
		for (size_t i = 0; i < locales.size(); ++i)
		{
			std::string index = std::to_string(i);

			UI::StringProperty(index.c_str(), locales[i], nullptr);
		}
	}

	UI::EndTreeNodeProperty(isLocalesTreeNodeOpen);

	const bool isIndicesTreeNodeOpen = UI::BeginTreeNodeProperty("indices", nullptr);

	if (isIndicesTreeNodeOpen)
	{
		for (size_t i = 0; i < indices.size(); ++i)
		{
			std::string index = std::to_string(i);

			UI::Property(index.c_str(), indices[i]);
		}
	}

	UI::EndTreeNodeProperty(isIndicesTreeNodeOpen);

	UI::EndProperties();

	End();
}

void MultiLanguagePanel::OnResourceLoaded()
{
	multiLanguageResource->Deserialize();
}
