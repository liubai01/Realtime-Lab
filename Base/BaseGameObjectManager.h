#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "BaseObject.h"
#include "BaseMainHeap.h"
#include "BaseRuntimeHeap.h"

using namespace std;

class BaseGameObjectManager
{
public:
	BaseGameObjectManager(ID3D12Device* device, BaseMainHeap* mainHeap);

	void DispatchTransformUpload(BaseRuntimeHeap* runtimeHeap);

	// uuid to pointer of objects
	unordered_map<string, shared_ptr<BaseObject>> mObjs;
	unordered_set<BaseObject*> mRootObjects;

	shared_ptr<BaseObject> CreateObject(const string& name);
	shared_ptr<BaseObject> GetObject(const string& uuid);

private:
	BaseMainHeap* mMainHeap;
	ID3D12Device* mDevice;
};

