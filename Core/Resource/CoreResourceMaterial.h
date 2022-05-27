#pragma once

#include "../../Base/Resource/BaseResource.h"
#include "../../Base/Resource/BaseResourceImage.h"

class CoreResourceMaterial : public BaseResource
{
public:
	static const BaseResourceType ClassResourceType = BaseResourceType::RESOURCE_MATERIAL;
	static const BaseAssetType ClassAssetType = BaseAssetType::ASSET_MATERIAL;

	string mName;
	BaseResourceImage* mDiffuseColorTexture;
	BaseResourceImage* mNormalMapTexture;

	CoreResourceMaterial(ID3D12Device* device, BaseAssetNode* assetNode);
	~CoreResourceMaterial();

	// handle override methods of BaseResource
	void Upload(ID3D12GraphicsCommandList* commandList);
	void RegisterMainHandle(BaseMainHeap* heap);
	void RegisterRuntimeHandle(BaseRuntimeHeap* heap);
};

