#include "UI/Panels/TextListPanel.h"
#include "Utility/FileDialog.h"
#include "Utility/ResourcePatcher.h"

TextListPanel::TextListPanel(const char* name, const char* icon, std::shared_ptr<TextList> textListResource) : BasePanel(name, icon)
{
    this->textListResource = textListResource;

    tableColumns.push_back({ "Name/Hash" , 0, 1.f });
    tableColumns.push_back({ "Text" , ImGuiTableColumnFlags_WidthStretch, 0.f });
}

void TextListPanel::Render()
{
	if (!Begin())
	{
		return;
	}

	if (!textListResource->IsResourceDeserialized())
	{
		ImGui::SetCursorPos(ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y / 2));
		ImGui::Text("Loading text list...");
		End();

		return;
	}

	if (ImGui::Button("Import JSON"))
	{
		std::string filePath = FileDialog::OpenFile("JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0");
		if (!filePath.empty())
		{
			textListResource->ImportFromJson(filePath);
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Export RAW"))
	{
		std::string filePath = FileDialog::SaveFile("RAW Files (*.raw)\0*.raw\0All Files (*.*)\0*.*\0");
		if (!filePath.empty())
		{
			textListResource->ExportRawData(filePath);
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Patch Back to Game"))
	{
		textListResource->SerializeToBuffer(); // Ensure buffer is up to date
		
		std::string resourceLibPath = textListResource->GetResourceLibraryFilePath();
		std::string headerLibPath = textListResource->GetHeaderLibraryFilePath();
		unsigned int offsetInResLib = textListResource->GetOffsetInResourceLibrary();
		unsigned int offsetInHeaderLib = textListResource->GetOffsetInHeaderLibrary();
		const void* newData = textListResource->GetResourceData();
		unsigned int newDataSize = textListResource->GetResourceDataSize();

		if (ResourcePatcher::PatchResourceLibrary(resourceLibPath, headerLibPath, offsetInResLib, offsetInHeaderLib, newData, newDataSize))
		{
			// Success
		}
	}

	ImGui::Separator();


	if (!UI::BeginProperties("TextListEntries", tableColumns))
	{
		End();

		return;
	}

	std::vector<TextList::Entry>& entries = textListResource->GetEntries();

	for (size_t i = 0; i < entries.size(); ++i)
	{
		UI::StringProperty(entries[i].name, entries[i].text);
	}

	UI::EndProperties();

	End();
}

void TextListPanel::OnResourceLoaded()
{
	textListResource->Deserialize();
}
