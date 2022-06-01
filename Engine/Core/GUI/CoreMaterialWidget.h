#pragma once
#include "../../Base/BaseGUIWidget.h"
#include "../../Base/Resource/BaseResourceManager.h"
#include "../../Base/Asset/BaseAssetManager.h"
#include "../Resource/CoreResourceMaterial.h"

class CoreMaterialWidget : public BaseGUIWidget
{
public:
	CoreMaterialWidget(BaseResourceManager* resourceManager, BaseAssetManager* assetManager);

	void AssignMat(CoreResourceMaterial* mat);

	void Update();
	void Start(ImGuiID& dockspace_id);

	bool mSetFocusThisFrame;
private:
	BaseResourceManager* mResourceManager;
	BaseAssetManager* mAssetManager;

	bool mIsBaseColorTextured;
	bool mIsNormalMapTextured;
	bool mIsMetallicTextured;
	bool mIsRoughnessTextured;


	std::string mMatResourceUUID;
};

