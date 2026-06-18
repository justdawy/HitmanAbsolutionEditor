#include "UI/Panels/SettingsPanel.h"
#include "Settings.h"
#include "Editor.h"
#include "Utility/FileDialog.h"
#include <shlobj.h>
#include <filesystem>
#include <string>

SettingsPanel::SettingsPanel(const char* name, const char* icon) : BasePanel(name, icon, false)
{
}

void SettingsPanel::Render()
{
	if (!Begin())
	{
		return;
	}

	if (ImGui::IsWindowAppearing()) {
		ImGui::SetWindowFocus();
	}

	Settings& settings = Settings::GetInstance();
	bool rpc = settings.GetEnableDiscordRPC();
	if (ImGui::Checkbox("Discord RPC", &rpc)) {
		settings.SetEnableDiscordRPC(rpc);
		settings.UpdateIniFile("EnableDiscordRPC", rpc ? "1" : "0");
	}

	if (ImGui::Button("Background")) {
		std::string path = FileDialog::OpenFile("Image Files (*.png;*.jpg;*.jpeg;*.bmp)\0*.png;*.jpg;*.jpeg;*.bmp\0All Files (*.*)\0*.*\0");
		if (!path.empty()) {
			PWSTR docsPath = NULL;
			std::string destPath = path;
			if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &docsPath))) {
				std::filesystem::path docsDir = docsPath;
				CoTaskMemFree(docsPath);
				std::filesystem::path ext = std::filesystem::path(path).extension();
				std::filesystem::path dest = docsDir / ("bg_hae" + ext.string());
				try {
					std::filesystem::copy_file(path, dest, std::filesystem::copy_options::overwrite_existing);
					destPath = dest.string();
				} catch (...) {
				}
			}
			settings.SetBackgroundImagePath(destPath);
			settings.UpdateIniFile("BackgroundImagePath", destPath);
			Editor::GetInstance().LoadBackgroundImage(destPath);
		}
	}

	Editor::GetInstance().SetSettingsPanelFocused(ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows));

	End();
}
