#pragma once
#include "../../Base/BaseGUIWidget.h"
#include "../../Base/BaseObject.h"

class CoreInspectorWidget :
    public BaseGUIWidget
{
public:
	CoreInspectorWidget(BaseObject** nowActiveObjectPtr, BaseResourceManager* resourceManager, BaseAssetManager* assetManager);

	void Start(ImGuiID& dockspace_id);
	void Update();
private:
	BaseObject** mNowActiveObjectPtr;

	BaseResourceManager* mResourceManager;
	BaseAssetManager* mAssetManager;

	void UpdateTransformGUI(BaseObject* obj);
};

