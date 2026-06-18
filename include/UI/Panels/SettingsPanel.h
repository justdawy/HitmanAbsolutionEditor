#pragma once
#include "BasePanel.h"
class SettingsPanel : public BasePanel
{
public:
	SettingsPanel(const char* name, const char* icon);
	void Render() override;
};