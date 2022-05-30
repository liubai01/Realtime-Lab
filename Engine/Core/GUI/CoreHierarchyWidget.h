#pragma once
#include "../../Base/BaseGUIWidget.h"
#include "../../Base/BaseGameObjectManager.h"

class CoreHierarchyWidget : public BaseGUIWidget
{
public:
	CoreHierarchyWidget(BaseGameObjectManager* GOManager, BaseObject** nowSelectObjectPtr);
	void Update();
	void Start(ImGuiID& dockspace_id);
private:
	BaseGameObjectManager* mGOManager;
	BaseObject** mNowSelectObjectPtr;
};

