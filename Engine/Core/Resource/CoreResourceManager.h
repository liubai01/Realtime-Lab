#pragma once

#include "../../Base/Resource/BaseResourceManager.h"
#include "./CoreResourceMaterial.h"

class CoreResourceManager: public BaseResourceManager
{
public:
	CoreResourceManager(ID3D12Device* device, BaseAssetManager* assetManager, BaseMainHeap* mainHeap);

	CoreResourceMaterial* CreateMaterial(const string& url);
	void SaveMaterial(CoreResourceMaterial* mat);
};

