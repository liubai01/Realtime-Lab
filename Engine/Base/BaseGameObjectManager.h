#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "BaseObject.h"
#include "BaseMainHeap.h"
#include "BaseRuntimeHeap.h"

class BaseGameObjectManager
{
public:
	BaseGameObjectManager(ID3D12Device* device, BaseMainHeap* mainHeap);

	void DispatchTransformUpload(BaseRuntimeHeap* runtimeHeap);

	// uuid to pointer of objects
	std::unordered_map<std::string, std::shared_ptr<BaseObject>> mObjs;
	std::unordered_set<BaseObject*> mRootObjects;

	std::shared_ptr<BaseObject> CreateObject(const std::string& name);
	std::shared_ptr<BaseObject> GetObject(const std::string& uuid);

private:
	BaseMainHeap* mMainHeap;
	ID3D12Device* mDevice;
};

