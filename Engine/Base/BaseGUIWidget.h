#pragma once

#include "../ThirdParty/ImGUI/imgui_internal.h"
#include <string>

class BaseGUIWidget
{
public:
	bool mIsShow;
	std::string mDisplayName;

	virtual void Start(ImGuiID& dockspace_id) = 0;
	virtual void Update() = 0;
};

