#pragma once
#include <windows.h>
#include "../ThirdParty/d3dx12.h"
#include <wrl.h>
#include <vector>
#include <dxgi1_4.h>
#include <unordered_map>
#include <memory>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "../DebugOut.h"
#include "../MathUtils.h"
#include <time.h>
#include "BaseStagedBuffer.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

using Microsoft::WRL::ComPtr;
using namespace std;

struct BaseCameraConstant {
  XMFLOAT4X4 ViewProj = Identity4x4();
  XMFLOAT4 EyePos = { 0.0f, 0.0f, 0.0f, 1.0f };
};

class BaseCamera: public BaseStagedBuffer<BaseCameraConstant>
{
public:
  BaseCamera(ID3D12Device* device, float width, float height, float FovAngleY=0.25f * 3.1415925f, float nearZ=1.0f, float FarZ=1000.0f);

  D3D12_VIEWPORT mViewport;    // area that output from rasterizer will be stretched to.
  D3D12_RECT mScissorRect;     // the area to draw in. pixels outside that area will not be drawn onto

  XMFLOAT4X4 mProj;
  XMFLOAT4 mPos;

  void SetPos(float x, float y, float z);

  float mWidth;
  float mHeight;

  void Upload();
};

