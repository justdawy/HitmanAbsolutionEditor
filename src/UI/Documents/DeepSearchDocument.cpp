#include <IconsMaterialDesignIcons.h>

#include "UI/Documents/DeepSearchDocument.h"
#include "UI/Panels/DeepSearchPanel.h"

DeepSearchDocument::DeepSearchDocument(const char* name, const char* icon, const Type type, const ImGuiID dockID) : Document(name, icon, type, -1, true, dockID)
{
	std::shared_ptr<DeepSearchPanel> deepSearchPanel = std::make_shared<DeepSearchPanel>("Deep Search", ICON_MDI_MAGNIFY);

	AddPanel(deepSearchPanel);
}

void DeepSearchDocument::CreateLayout(const ImGuiID dockspaceID, const ImVec2 dockspaceSize)
{
	ImGui::DockBuilderDockWindow(CalculatePanelID(0, currentDockspaceID).c_str(), dockspaceID);
	ImGui::DockBuilderFinish(dockspaceID);
}
