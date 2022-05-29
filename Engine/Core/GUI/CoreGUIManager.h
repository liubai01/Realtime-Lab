#pragma once


#include "../../Base/BaseGUIWidget.h"
#include "../../Base/BaseApp.h"
#include "CoreHierarchyWidget.h"
#include "../Resource/CoreResourceManager.h"
#include<vector>

class CoreGUIManager
{
public:

	CoreGUIManager(BaseApp* app, CoreResourceManager* resourceManager);

	void Render(ID3D12GraphicsCommandList* commandList);
	void Update();
	void Start();

	std::vector<std::unique_ptr<BaseGUIWidget>> mWidgets;
	BaseObject* mNowSelectedObject;

private:
	bool mFirstLoop;
	BaseApp* mApp;
	CoreResourceManager* mResourceManager;
};

