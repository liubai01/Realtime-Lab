#pragma once
#include "BaseResourceImage.h"
#include "../AssetManager/BaseAssetManager.h"
#include <memory>
#include <unordered_map>
#include "string"

using namespace std;

class BaseResourceManager
{
public:
	BaseResourceManager(ID3D12Device* device, BaseAssetManager* assetManager, BaseMainHeap* mainHeap);
	~BaseResourceManager();

	BaseResourceImage* LoadImage(const string& url);

	void Upload(ID3D12GraphicsCommandList* commandList);
	void RegisterRuntimeHeap(BaseRuntimeHeap* runtimeHeap);
private:
	ID3D12Device* mDevice;
	BaseAssetManager* mAssetManager;

	BaseMainHeap* mMainHeap;

	unordered_map<string, BaseResource*> mUrl2ResourcePtr;
};

