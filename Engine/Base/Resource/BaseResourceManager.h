#pragma once
#include "BaseResourceImage.h"
#include "BaseResourceMesh.h"
#include "../Asset/BaseAssetManager.h"
#include <memory>
#include <unordered_map>
#include "string"

class BaseResourceManager
{
public:
	BaseResourceManager(ID3D12Device* device, BaseAssetManager* assetManager, BaseMainHeap* mainHeap);
	virtual ~BaseResourceManager();

	template<class T>
	T* LoadByURL(const std::string& url);

	template<class T>
	T* LoadByUUID(const std::string& uuid);

	void Upload(ID3D12GraphicsCommandList* commandList);
	void RegisterRuntimeHeap(BaseRuntimeHeap* runtimeHeap);

	ID3D12Device* mDevice;
	BaseAssetManager* mAssetManager;

	BaseMainHeap* mMainHeap;
	std::unordered_map<std::string, BaseResource*> mAssetUUID2ResourcePtr;
	std::unordered_map<std::string, BaseResource*> mResourceUUID2ResourcePtr;
};

template<class T>
T* BaseResourceManager::LoadByURL(const std::string & url)
{
	T* ret;

	// check whether that file is registered in asset manager
	BaseAssetNode* node = mAssetManager->LoadAsset(url);
	// if the asset is not registed at asset manager, return nullptr
	if (!node)
	{
		return nullptr;
	}
	// check whether that asset is an image
	if (node->GetAssetType() != T::ClassAssetType)
	{
		return nullptr;
	}

	std::unordered_map<std::string, BaseResource*>::iterator f = mAssetUUID2ResourcePtr.find(node->mUUID);
	// check whether this resource has been requestsed (and cached in mUrl2ResourcePtr)
	if (f != mAssetUUID2ResourcePtr.end())
	{
		// check whether url points to a resources is an image
		if (f->second->mType == T::ClassResourceType)
		{
			return dynamic_cast<T*>(f->second);
		}
		// url points to a resources that is not expected resource type
		return nullptr;
	}

	// initialize the resource
	ret = new T(
		mDevice,
		node
	);
	
	mAssetUUID2ResourcePtr[node->mUUID] = ret;
	mResourceUUID2ResourcePtr[ret->mUUIDResource] = ret;

	// register on the main heap
	if (ret->mIsRuntimeResource)
	{
		ret->RegisterMainHandle(mMainHeap);
	}
	return ret;
}

template<class T>
T* BaseResourceManager::LoadByUUID(const std::string& uuid)
{
	std::unordered_map<std::string, BaseResource*>::iterator f = mResourceUUID2ResourcePtr.find(uuid);
	// check whether this resource has been requestsed (and cached in mUrl2ResourcePtr)
	if (f != mResourceUUID2ResourcePtr.end())
	{
		// check whether url points to a resources is an image
		if (f->second->mType == T::ClassResourceType)
		{
			return dynamic_cast<T*>(f->second);
		}
		// url points to a resources that is not expected resource type
		return nullptr;
	}

	// not found
	return nullptr;
}
