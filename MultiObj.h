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
  XMFLOAT3 normal;
  XMFLOAT2 texCoord;
};

struct GlobalConsts {
  XMFLOAT4X4 WorldViewProj = Identity4x4();
  XMFLOAT4X4 World = Identity4x4();
  XMFLOAT4X4 RSInvT = Identity4x4();
  XMFLOAT4 LightDir = {};
  XMFLOAT4 EyePos = {};
};

struct BaseMaterialConsts {
  XMFLOAT4 Ka = { 0.6f, 0.6f, 0.6f, 1.0f }; // Ambient Color Constant
  XMFLOAT4 Kd = { 0.8f, 0.8f, 0.8f, 1.0f }; // Diffuse Color Constant
  XMFLOAT4 Ks = { 0.5f, 0.5f, 0.5f, 1.0f }; // Specular Color Constant
  float Ns = 323.999994f; // Shiniess
  bool IsTextured = false; // Whether applied texture
};

void SetCubeGeo(BaseGeometry<Vertex>& geo);

class MyApp : public BaseApp {
public:
  vector<Vertex> mvList;
  MyApp(HINSTANCE hInstance);

  void Start();
  void Render();
  void Update();

  ComPtr<ID3D12DescriptorHeap> mDescHeap;

  unique_ptr<BaseUploadHeap<GlobalConsts>> mConstBuffer;
  unique_ptr<BaseUploadHeap<BaseMaterialConsts>> mMatBuffer;
  unique_ptr<BaseImage> mTexture;

  unique_ptr<BaseDrawContext> mDrawContext;

  void InitConstBuffer();
  void InitTextureBuffer();
  void InitMatBuffer();
  void LoadObj();

};
