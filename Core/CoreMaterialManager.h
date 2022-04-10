#pragma once
#include "CoreMaterial.h"
#include <unordered_map>
#include <memory>
#include <string>
#include "../Base/BaseRuntimeHeap.h"

using namespace std;

class CoreMaterialManager
{
public:
	CoreMaterialManager(ID3D12Device* device);

	shared_ptr<CoreMaterial> CreateMaterial(const string& name);
	shared_ptr<CoreMaterial> GetMaterial(const string& name);

	void RegisterRuntimeHandle(BaseRuntimeHeap* runtimeHeap);

private:
	unordered_map<string, shared_ptr<CoreMaterial>> mName2Mat;
	ID3D12Device* mDevice;
};

