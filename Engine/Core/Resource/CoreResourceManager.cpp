#include "CoreResourceManager.h"
#include "CoreResourceMaterial.h"

CoreResourceManager::CoreResourceManager(ID3D12Device* device, BaseAssetManager* assetManager, BaseMainHeap* mainHeap): BaseResourceManager(device, assetManager, mainHeap)
{
	// load default material
	LoadByURL<CoreResourceMaterial>("EditorAsset\\Default.mat");
}

CoreResourceMaterial* CoreResourceManager::CreateMaterial(const std::string& url)
{
	CoreResourceMaterial* ret = nullptr;

	std::string postfix = url.substr(url.find_last_of(".") + 1);
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

	std::string toPath = mAssetManager->mRootPath + "\\" + url;
	
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

void CoreResourceManager::SaveMaterial(CoreResourceMaterial* mat)
{

}