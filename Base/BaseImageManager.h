#pragma once
#include "BaseMainHeap.h"
#include "BaseImage.h"
#include "BaseUUIDGenerator.h"

class BaseImageManager
{
public:
	BaseImageManager(ID3D12Device* device, BaseMainHeap* mainHeap);
	shared_ptr<BaseImage> CreateImage(string filepath, string name);
	void RegisterRuntimeHandle(BaseRuntimeHeap* runtimeHeap);
	void Upload(ID3D12GraphicsCommandList* commandList);
private:
	unordered_map<string, shared_ptr<BaseImage>> mUUID2Image;
	BaseMainHeap* mMainHeap;
	ID3D12Device* mDevice;
};

