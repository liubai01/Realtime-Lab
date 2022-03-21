#pragma once
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "../ThirdParty/d3dx12.h"
#include <wrl.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class BaseTransform
{
  XMFLOAT4 mPosition = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
  XMFLOAT4 mRotation = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
  XMFLOAT4 mScale = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f);

  void SetPos(float x, float y, float z);
  void SetRot(float x, float y, float z);
  void SetScale(float x, float y, float z);

  XMMATRIX GetWorldMatrix();
  XMMATRIX GetRSInvT();
};

