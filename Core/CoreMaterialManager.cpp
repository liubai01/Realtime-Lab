#include "CoreMaterialManager.h"
#include "../Base/BaseUUIDGenerator.h"

CoreMaterialManager::CoreMaterialManager(ID3D12Device* device, BaseMainHeap* mainHeap) 
{
	mDevice = device;
	mMainHeap = mainHeap;
}

void CoreMaterialManager::RegisterRuntimeHandle(BaseRuntimeHeap* runtimeHeap)
{
	for (auto i : mUUID2Mat)
	{
		i.second->RegisterRuntimeHandle(runtimeHeap);
	}
}

shared_ptr<CoreMaterial> CoreMaterialManager::CreateMaterial(const string& name)
{
	shared_ptr<CoreMaterial> ret;
	string matUuid = uuid::generate_uuid();
	auto f = mUUID2Mat.find(matUuid);

	if (f == mUUID2Mat.end())
	{
		ret = make_shared<CoreMaterial>(mDevice);
		mUUID2Mat[matUuid] = ret;
		ret->mName = name;
		ret->RegisterMainHandle(mMainHeap);
	} else {
		dout::printf("[CoreMaterialManager] Repeated Material %s!", matUuid);
		ret = f->second;
	}
	return ret;
}

shared_ptr<CoreMaterial> CoreMaterialManager::GetMaterial(const string& uuid)
{
	shared_ptr<CoreMaterial> ret = nullptr;
	auto f = mUUID2Mat.find(uuid);

	if (f != mUUID2Mat.end())
	{
		ret = f->second;
	} else {
		dout::printf("[CoreMaterialManager] Material %s not found!", uuid);
	}

	return ret;
}