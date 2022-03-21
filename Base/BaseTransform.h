#pragma once
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "../ThirdParty/d3dx12.h"
#include "BaseUploadHeap.h"
#include "BaseMainHeap.h"
#include <wrl.h>
#include "../MathUtils.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct BaseTransformConstant {
    XMFLOAT4X4 World = Identity4x4();
    XMFLOAT4X4 RSInvT = Identity4x4();
};

class BaseTransform
{
public:
  BaseTransform(ID3D12Device* device);

  void SetPos(float x, float y, float z);
  void SetRot(float x, float y, float z);
  void SetScale(float x, float y, float z);

  XMMATRIX GetWorldMatrix();
  XMMATRIX GetRSInvT();

  BaseDescHeapHandle GetTransformHandle();
  void RegisterHandle(BaseMainHeap* heap);


  XMFLOAT4 mPosition = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
  XMFLOAT4 mRotation = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
  XMFLOAT4 mScale = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f);
private:

  BaseUploadHeap<BaseTransformConstant> mBuffer;
};

