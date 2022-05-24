#include "BaseImageManager.h"

BaseImageManager::BaseImageManager(ID3D12Device* device, BaseMainHeap* mainHeap)
{
	mMainHeap = mainHeap;
	mDevice = device;
}

BaseImageManager::~BaseImageManager()
{
	for (auto i : mUUID2Image)
	{
		mMainHeap->FreeHeapHandle(i.second->mMainHandle);
	}
	
}

shared_ptr<BaseImage> BaseImageManager::CreateImage(string filepath, string name)
{
	shared_ptr<BaseImage> ret;
	string matUuid = uuid::generate_uuid();
	auto f = mUUID2Image.find(matUuid);

	if (f == mUUID2Image.end())
	{
		ret = make_shared<BaseImage>(mDevice, filepath, name);
		mUUID2Image[matUuid] = ret;
		ret->mName = name;
		ret->RegisterMainHandle(mMainHeap);
	}
	else {
		dout::printf("[BaseImageManager] Repeated Material %s!", matUuid);
		ret = f->second;
	}
	return ret;
}

void BaseImageManager::RegisterRuntimeHandle(BaseRuntimeHeap* runtimeHeap)
{
	for (auto i : mUUID2Image)
	{
		i.second->RegisterRuntimeHandle(runtimeHeap);
	}
}

void BaseImageManager::Upload(ID3D12GraphicsCommandList* commandList)
{
	for (auto i : mUUID2Image)
	{
		if (!i.second->mIsUploaded)
		{
			i.second->Upload(commandList);
		}
		
	}
}