#include "BaseResourceManager.h"

BaseResourceManager::BaseResourceManager(ID3D12Device* device, BaseAssetManager* assetManager, BaseMainHeap* mainHeap)
{
	mAssetManager = assetManager;
	mDevice = device;
	mUrl2ResourcePtr.clear();

	mMainHeap = mainHeap;
}

BaseResourceImage* BaseResourceManager::LoadImage(const string& url)
{
	BaseResourceImage* ret;
	unordered_map<string, BaseResource*>::iterator f = mUrl2ResourcePtr.find(url);
	// check whether this resource has been requestsed (and cached in mUrl2ResourcePtr)
	if (f != mUrl2ResourcePtr.end())
	{
		// check whether url points to a resources is an image
		if (f->second->mType == BaseResourceType::RESOURCE_IMAGE)
		{
			return dynamic_cast<BaseResourceImage*>(f->second);
		}
		// url points to a resources that is not an image
		return nullptr;
	}

	// check whether that file is registered in asset manager
	BaseAssetNode* node = mAssetManager->LoadAsset(url);
	// if the asset is not registed at asset manager, return nullptr
	if (!node)
	{
		return nullptr;
	}
	// check whether that asset is an image
	if (node->GetAssetType() != BaseAssetType::ASSET_IMAGE)
	{
		return nullptr;
	}

	// initialize the image
	ret = new BaseResourceImage(
		mDevice, 
		mAssetManager->GetAssetFullPath(node)
	);
	
	mUrl2ResourcePtr[url] = ret;
	
	// register on the main heap
	ret->RegisterMainHandle(mMainHeap);
	return ret;
}

BaseResourceMesh* BaseResourceManager::LoadMesh(const string& url)
{
	BaseResourceMesh* ret;
	unordered_map<string, BaseResource*>::iterator f = mUrl2ResourcePtr.find(url);
	// check whether this resource has been requestsed (and cached in mUrl2ResourcePtr)
	if (f != mUrl2ResourcePtr.end())
	{
		// check whether url points to a resources is a mesh
		if (f->second->mType == BaseResourceType::RESOURCE_MESH)
		{
			return dynamic_cast<BaseResourceMesh*>(f->second);
		}
		// url points to a resources that is not an image
		return nullptr;
	}

	// check whether that file is registered in asset manager
	BaseAssetNode* node = mAssetManager->LoadAsset(url);
	// if the asset is not registed at asset manager, return nullptr
	if (!node)
	{
		return nullptr;
	}
	// check whether that asset is an object
	if (node->GetAssetType() != BaseAssetType::ASSET_OBJ)
	{
		return nullptr;
	}

	// initialize the model
	ret = new BaseResourceMesh(
		mDevice,
		mAssetManager->GetAssetFullPath(node)
	);

	mUrl2ResourcePtr[url] = ret;
	return ret;
}

void BaseResourceManager::Upload(ID3D12GraphicsCommandList* commandList)
{
	for (auto& item : mUrl2ResourcePtr)
	{
		if (!item.second->mUpload)
		{
			item.second->Upload(commandList);
	
		}
	}
}

void BaseResourceManager::RegisterRuntimeHeap(BaseRuntimeHeap* runtimeHeap)
{
	for (auto& item : mUrl2ResourcePtr)
	{
		item.second->RegisterRuntimeHandle(runtimeHeap);
	}
}

BaseResourceManager::~BaseResourceManager()
{
	// remark: all the resources allocated by resource manager
	//         should not released manually
	for (auto& item: mUrl2ResourcePtr)
	{
		BaseResource* resource = item.second;
		// resource image gets the runtime handle
		if (resource->mType == BaseResourceType::RESOURCE_IMAGE)
		{
			mMainHeap->FreeHeapHandle(resource->mMainHandle);
		}
		delete resource;
		item.second = nullptr;
	}
}