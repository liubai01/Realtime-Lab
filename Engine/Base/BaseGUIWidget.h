#pragma once

#include "../ThirdParty/ImGUI/imgui_internal.h"

class BaseGUIWidget
{
public:
	virtual void Start(ImGuiID& dockspace_id) = 0;
	virtual void Update() = 0;
};

