#pragma once

#include "UI/Documents/Document.h"

class DeepSearchDocument : public Document
{
public:
	DeepSearchDocument(const char* name, const char* icon, const Type type, const ImGuiID dockID = 0);
	void CreateLayout(const ImGuiID dockspaceID, const ImVec2 dockspaceSize) override;
};
