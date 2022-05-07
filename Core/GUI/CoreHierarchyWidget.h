#pragma once
#include "../../Base/BaseGUIWidget.h"
#include "../../Base/BaseGameObjectManager.h"

class CoreHierarchyWidget : public BaseGUIWidget
{
public:
	CoreHierarchyWidget(BaseGameObjectManager* GOManager);
	void UpdateGUI();
private:
	BaseGameObjectManager* mGOManager;
};

