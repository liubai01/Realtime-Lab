#pragma once
#include "../../Base/BaseGUIWidget.h"
#include "../../Base/BaseGameObjectManager.h"
#include "../../Base/AssetManager/BaseAssetManager.h"
#include "../../Base/Resource/BaseResourceManager.h"

class CoreAssetWidget : public BaseGUIWidget
{
public:
	CoreAssetWidget(BaseAssetManager* assetManager, BaseResourceManager* resourceManager);
	void Update();
	void Start(ImGuiID& dockspace_id);

private:
	void UpdateAssetContentWindow();
	void UpdateAssetExplorerWindow();
	void DFSConstructExploererNode(BaseAssetNode* nowNode);

	BaseAssetManager* mAssetManager;
	BaseResourceManager* mResourceManager;

	BaseAssetNode* mNowSelectedAssetNodeDir;
};

