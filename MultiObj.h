#pragma once
#include <windows.h>
#include "DebugOut.h"
#include "BaseApp.h"
#include "BaseGeometry.h"
#include "BaseUploadHeap.h"
#include "BaseImage.h"
#include <memory>

using namespace std;

struct Vertex {
  XMFLOAT3 pos;
  XMFLOAT4 color;
  XMFLOAT3 normal;
  XMFLOAT2 texCoord;
};

struct GlobalConsts {
  XMFLOAT4X4 WorldViewProj = Identity4x4();
  XMFLOAT4X4 World = Identity4x4();
  XMFLOAT4X4 RSInvT = Identity4x4();
  XMFLOAT4 LightDir = {};
  XMFLOAT4 EyePos = {};
  XMFLOAT4 TexBlend = {};
};

void SetCubeGeo(BaseGeometry<Vertex>& geo, XMFLOAT4& color);

class MyApp : public BaseApp {
public:
  vector<Vertex> mvList;
  MyApp(HINSTANCE hInstance);

  void Start();
  void Render();
  void Update();

  ComPtr<ID3D12DescriptorHeap> mDescHeap;
  D3D12_DESCRIPTOR_RANGE mDescTableRanges[1];

  unique_ptr<BaseUploadHeap<GlobalConsts>> mConstBuffer;
  unique_ptr<BaseImage> mTexture;

  unique_ptr<BaseDrawContext> mDrawContext;

  void InitConstBuffer();
  void InitTextureBuffer();
  void LoadObj();

};
