#include "CoreMaterialManager.h"

CoreMaterialManager::CoreMaterialManager(ID3D12Device* device) 
{
	mDevice = device;
}

void CoreMaterialManager::RegisterRuntimeHandle(BaseRuntimeHeap* runtimeHeap)
{
	for (auto i : mName2Mat)
	{
		i.second->RegisterRuntimeHandle(runtimeHeap);
	}
}

shared_ptr<CoreMaterial> CoreMaterialManager::CreateMaterial(const string& name)
{
	shared_ptr<CoreMaterial> ret;
	auto f = mName2Mat.find(name);

	if (f == mName2Mat.end())
	{
		ret = make_shared<CoreMaterial>(mDevice);
		mName2Mat[name] = ret;
	} else {
		dout::printf("[CoreMaterialManager] Repeated Material %s!", name);
		ret = f->second;
	}

	return ret;
}

shared_ptr<CoreMaterial> CoreMaterialManager::GetMaterial(const string& name)
{
	shared_ptr<CoreMaterial> ret = nullptr;
	auto f = mName2Mat.find(name);

	if (f != mName2Mat.end())
	{
		ret = f->second;
	} else {
		dout::printf("[CoreMaterialManager] Material %s not found!", name);
	}

	return ret;
}