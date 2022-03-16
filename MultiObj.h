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
  XMFLOAT4X4 ShadowViewProj = Identity4x4();
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
  MyApp(HINSTANCE hInstance);

  void Start();
  void Render();
  void Update();

  void RenderShadowMap();
  void RenderObjects();
  void MatrixBuild(XMVECTOR& viewPos, XMMATRIX& proj);

  ComPtr<ID3D12DescriptorHeap> mDescHeap;

  unique_ptr<BaseUploadHeap<GlobalConsts>> mConstBuffer;
  unique_ptr<BaseUploadHeap<BaseMaterialConsts>> mMatBuffer;
  unique_ptr<BaseImage> mTexture;

  unique_ptr<BaseDrawContext> mDrawContext;

  XMVECTOR mLightDir;
  XMVECTOR mViewPos;
  XMVECTOR mSMPos;
  
  // Shadow map depth buffer
  ComPtr<ID3D12Resource> mShadowDepthStencilBuffer;
  ComPtr<ID3D12DescriptorHeap> mShadowDsDescriptorHeap;

  D3D12_CPU_DESCRIPTOR_HANDLE ShadowDepthBufferView() const;
  void InitShadowDepth();

  void InitConstBuffer();
  void InitTextureBuffer();
  void InitMatBuffer();
  void LoadObj();

};
