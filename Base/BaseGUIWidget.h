#pragma once

#include "../ThirdParty/ImGUI/imgui_internal.h"

class BaseGUIWidget
{
public:
	virtual void UpdateGUI() = 0;
};

