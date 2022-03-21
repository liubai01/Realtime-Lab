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

using namespace DirectX;
using Microsoft::WRL::ComPtr;
using namespace std;

class BaseObject
{
public:
  BaseObject(const string& name, ID3D12Device* device) : mName(name), mTransform(device) {
  }

  string mName;
  BaseTransform mTransform;

  void AddComponent(shared_ptr<BaseComponent> component);
  vector<shared_ptr<BaseComponent>> mComponents;
};

