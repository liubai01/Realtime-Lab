#pragma once
#include "../BaseDescHeapHandle.h"
#include "../BaseMainHeap.h"
#include "../BaseRuntimeHeap.h"
#include "../AssetManager/BaseAssetNode.h"

enum class BaseResourceType {
	RESOURCE_IMAGE,
	RESOURCE_MESH,
	RESOURCE_MATERIAL,
	RESOURCE_UNKNOWN
};

class BaseResource
{
public:
	static const BaseResourceType ClassResourceType = BaseResourceType::RESOURCE_UNKNOWN;
	static const BaseAssetType ClassAssetType = BaseAssetType::ASSET_UNKNOWN;

	BaseResource(ID3D12Device* device, BaseAssetNode* assetNode);
	virtual ~BaseResource() { };

	string mName;
	// the UUID of this resource
	// Remark: one asset could have multiple instances of resource
	string mUUIDResource;
	// the UUID of corresponding asset (used for serialization)
	string mUUIDAsset;
	BaseResourceType mType;

	// whether the resource needs being registered, by default is false
	bool mIsRuntimeResource;

	bool mUpload;

	virtual void RegisterMainHandle(BaseMainHeap* heap) = 0;
	virtual void RegisterRuntimeHandle(BaseRuntimeHeap* heap) = 0;

	BaseDescHeapHandle mMainHandle;
	BaseDescHeapHandle mRuntimeHandle;

	virtual void Upload(ID3D12GraphicsCommandList* commandList) = 0;

	// The device of image would be allocated to
	ID3D12Device* mDevice;
};

