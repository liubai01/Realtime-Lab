#pragma once
#include "../../Base/BaseGUIWidget.h"
#include "../../Base/BaseObject.h"

class CoreInspectorWidget :
    public BaseGUIWidget
{
public:
	CoreInspectorWidget(BaseObject** nowSelectObjectPtr);
	void Start(ImGuiID& dockspace_id);
	void Update();
private:
	BaseObject** mNowSelectObjectPtr;
};

