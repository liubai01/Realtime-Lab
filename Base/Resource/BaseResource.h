#pragma once
#include "../BaseDescHeapHandle.h"
#include "../BaseMainHeap.h"
#include "../BaseRuntimeHeap.h"

enum class BaseResourceType {
	RESOURCE_IMAGE,
	RESOURCE_UNKNOWN
};

class BaseResource
{
public:
	BaseResource(ID3D12Device* device, const string& path);
	virtual ~BaseResource() { };

	string mName;
	string mUUID;
	BaseResourceType mType;
	bool mUpload;

	virtual void RegisterMainHandle(BaseMainHeap* heap) = 0;
	virtual void RegisterRuntimeHandle(BaseRuntimeHeap* heap) = 0;

	BaseDescHeapHandle mMainHandle;
	BaseDescHeapHandle mRuntimeHandle;

	virtual void Upload(ID3D12GraphicsCommandList* commandList) = 0;

	// The device of image would be allocated to
	ID3D12Device* mDevice;
};

