#pragma once
#include "../BaseDescHeapHandle.h"
#include "../BaseMainHeap.h"
#include "../BaseRuntimeHeap.h"
#include "../Asset/BaseAssetNode.h"

enum class BaseResourceType {
	RESOURCE_IMAGE,
	RESOURCE_MESH,
	RESOURCE_MATERIAL,
	RESOURCE_UNKNOWN
};

std::string BaseResourceType2String(enum class BaseResourceType& t);

class BaseResource
{
public:
	// type traits
	static const BaseResourceType ClassResourceType = BaseResourceType::RESOURCE_UNKNOWN;
	static const BaseAssetType ClassAssetType = BaseAssetType::ASSET_UNKNOWN;

	BaseResource(ID3D12Device* device, BaseAssetNode* assetNode);
	virtual ~BaseResource() { };

	std::string mName;
	// the UUID of this resource
	// Remark: one asset could have multiple instances of resource
	std::string mUUIDResource;
	// the UUID of corresponding asset (used for serialization)
	std::string mUUIDAsset;
	// for runtime type check of objects
	BaseResourceType mType;

	// whether the resource needs being registered&uploaded, by default is false
	bool mIsRuntimeResource;
	// whether the resource has been uploaded
	bool mUpload;

	virtual void RegisterMainHandle(BaseMainHeap* heap) = 0;
	virtual void RegisterRuntimeHandle(BaseRuntimeHeap* heap) = 0;

	virtual void ReleaseMainHandle(BaseMainHeap* heap);

	BaseDescHeapHandle mMainHandle;
	BaseDescHeapHandle mRuntimeHandle;

	virtual void Upload(ID3D12GraphicsCommandList* commandList) = 0;

	// The device of image would be allocated to
	ID3D12Device* mDevice;
};

