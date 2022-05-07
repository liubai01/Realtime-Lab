#pragma once


#include "../../Base/BaseGUIWidget.h"
#include "../../Base/BaseApp.h"
#include "CoreHierarchyWidget.h"
#include<vector>

using namespace std;

class CoreGUIManager
{
public:

	CoreGUIManager(BaseApp* app);

	void Render(ID3D12GraphicsCommandList* commandList);
	void Update();
	void Start();

	vector<unique_ptr<BaseGUIWidget>> mWidgets;
private:
	bool mFirstLoop;
	BaseApp* mApp;
};

