#pragma once
#include "CoreMaterial.h"
#include <unordered_map>
#include <memory>
#include <string>
#include "../Base/BaseRuntimeHeap.h"
#include "../Base/BaseMainHeap.h"

using namespace std;

class CoreMaterialManager
{
public:
	CoreMaterialManager(ID3D12Device* device, BaseMainHeap* mainHeap);

	shared_ptr<CoreMaterial> CreateMaterial(const string& name);
	shared_ptr<CoreMaterial> GetMaterial(const string& uuid);

	void RegisterRuntimeHandle(BaseRuntimeHeap* runtimeHeap);

private:
	unordered_map<string, shared_ptr<CoreMaterial>> mUUID2Mat;
	BaseMainHeap* mMainHeap;
	ID3D12Device* mDevice;
};

