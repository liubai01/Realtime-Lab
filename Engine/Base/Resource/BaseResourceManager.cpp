#include "BaseResourceManager.h"

BaseResourceManager::BaseResourceManager(ID3D12Device* device, BaseAssetManager* assetManager, BaseMainHeap* mainHeap)
{
	mAssetManager = assetManager;
	mDevice = device;
	mAssetUUID2ResourcePtr.clear();

	mMainHeap = mainHeap;
}

void BaseResourceManager::Upload(ID3D12GraphicsCommandList* commandList)
{
	for (auto& item : mAssetUUID2ResourcePtr)
	{
		if (!item.second->mUpload)
		{
			item.second->Upload(commandList);
	
		}
	}
}

void BaseResourceManager::RegisterRuntimeHeap(BaseRuntimeHeap* runtimeHeap)
{
	for (auto& item : mAssetUUID2ResourcePtr)
	{
		item.second->RegisterRuntimeHandle(runtimeHeap);
	}
}

BaseResourceManager::~BaseResourceManager()
{
	// remark: all the resources allocated by resource manager
	//         should not be released manually (a garbage collection system in the future)
	for (auto& item: mAssetUUID2ResourcePtr)
	{
		BaseResource* resource = item.second;
		// resource image gets the runtime handle
		if (resource)
		{
			resource->ReleaseMainHandle(mMainHeap);
			delete resource;
			item.second = nullptr;
		}

	}
}