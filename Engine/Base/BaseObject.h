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

class BaseObject
{
public:
  BaseObject(const std::string& name, const std::string& uuid, ID3D12Device* device, std::unordered_set<BaseObject*>* rootObjects) : mName(name), mTransform(device) {
	  this->mUuid = uuid;
	  this->mRootObjects = rootObjects;
  }

  BaseObject* GetParent();
  void SetParent(BaseObject* obj);
  const std::unordered_set<BaseObject*>& GetChildObjects();

  void DispatchTransformUpload(BaseRuntimeHeap* runtimeHeap);

  std::string mName;
  BaseTransform mTransform;

  void AddComponent(std::shared_ptr<BaseComponent> component);
  std::string GetUUID();
  std::vector<std::shared_ptr<BaseComponent>> mComponents;

private:
	std::string mUuid;

	BaseObject* mParentObject;
	std::unordered_set<BaseObject*> mChildObjects;
	std::unordered_set<BaseObject*>* mRootObjects;
};

