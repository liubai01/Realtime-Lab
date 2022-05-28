#pragma once

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "../ThirdParty/d3dx12.h"
#include <wrl.h>
#include "BaseGeometry.h"
#include "BaseTransform.h"
#include "Component/BaseComponent.h"
#include <string>
#include <memory>
#include <unordered_set>

using namespace DirectX;
using Microsoft::WRL::ComPtr;
using namespace std;

class BaseObject
{
public:
  BaseObject(const string& name, const string& uuid, ID3D12Device* device, unordered_set<BaseObject*>* rootObjects) : mName(name), mTransform(device) {
	  this->mUuid = uuid;
	  this->mRootObjects = rootObjects;
  }

  BaseObject* GetParent();
  void SetParent(BaseObject* obj);
  const unordered_set<BaseObject*>& GetChildObjects();

  void DispatchTransformUpload(BaseRuntimeHeap* runtimeHeap);

  string mName;
  BaseTransform mTransform;

  void AddComponent(shared_ptr<BaseComponent> component);
  string GetUUID();
  vector<shared_ptr<BaseComponent>> mComponents;

private:
	string mUuid;

	BaseObject* mParentObject;
	unordered_set<BaseObject*> mChildObjects;
	unordered_set<BaseObject*>* mRootObjects;
};

