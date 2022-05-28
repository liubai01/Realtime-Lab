#include "CoreResourceManager.h"


CoreResourceManager::CoreResourceManager(ID3D12Device* device, BaseAssetManager* assetManager, BaseMainHeap* mainHeap): BaseResourceManager(device, assetManager, mainHeap)
{

}

CoreResourceMaterial* CoreResourceManager::CreateMaterial(const string& url)
{
	CoreResourceMaterial* ret = nullptr;

	string postfix = url.substr(url.find_last_of(".") + 1);
	if (postfix != "mat")
	{
		dout::printf("[CoreResourceManager] Invalid material postfix: %s\n", postfix.c_str());
		return nullptr;
	}

	// check whether that file is registered in asset manager
	BaseAssetNode* node = mAssetManager->LoadAsset(url);
	// if the asset is registed at asset manager, return nullptr
	//      we could not overwrite current asset
	if (node)
	{
		dout::printf("[CoreResourceManager] Could not overwrite current material: %s\n", url.c_str());
		return nullptr;
	}

	string toPath = mAssetManager->mRootPath + "\\" + url;
	
	// Create an empty matereial
	CoreResourceMaterial::CreateEmpty(toPath);

	// Register into assetManager
	node = mAssetManager->RegisterAsset(url, toPath);
	
	// initialize the model
	ret = new CoreResourceMaterial(
		mDevice,
		node
	);

	ret->RegisterMainHandle(mMainHeap);
	mAssetUUID2ResourcePtr[node->mUUID] = ret;
	return ret;
}

CoreResourceMaterial* CoreResourceManager::LoadMaterial(const string& url)
{
	CoreResourceMaterial* ret;

	// check whether that file is registered in asset manager
	BaseAssetNode* node = mAssetManager->LoadAsset(url);
	// if the asset is not registed at asset manager, return nullptr
	if (!node)
	{
		return nullptr;
	}
	// check whether that asset is an object
	if (node->GetAssetType() != BaseAssetType::ASSET_MATERIAL)
	{
		return nullptr;
	}

	unordered_map<string, BaseResource*>::iterator f = mAssetUUID2ResourcePtr.find(node->mUUID);
	// check whether this resource has been requestsed (and cached in mUrl2ResourcePtr)
	if (f != mAssetUUID2ResourcePtr.end())
	{
		// check whether url points to a resources is a mesh
		if (f->second->mType == BaseResourceType::RESOURCE_MATERIAL)
		{
			return dynamic_cast<CoreResourceMaterial*>(f->second);
		}
		// url points to a resources that is not an image
		return nullptr;
	}

	// initialize the model
	ret = new CoreResourceMaterial(
		mDevice,
		node
	);

	mAssetUUID2ResourcePtr[node->mUUID] = ret;
	return ret;
}

void CoreResourceManager::SaveMaterial(CoreResourceMaterial* mat)
{

}