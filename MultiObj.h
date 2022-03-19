//#pragma once
//#include <windows.h>
//#include "DebugOut.h"
//#include "Base/BaseApp.h"
//#include "Base/BaseGeometry.h"
//#include "Base/BaseUploadHeap.h"
//#include "Base/BaseImage.h"
//#include <memory>
//
//#include "ThirdParty/ImGUI/imgui.h"
//#include "ThirdParty/ImGUI/imgui_impl_win32.h"
//#include "ThirdParty/ImGUI/imgui_impl_dx12.h"
//
//using namespace std;
//
//struct Vertex {
//  XMFLOAT3 pos;
//  XMFLOAT3 normal;
//  XMFLOAT2 texCoord;
//};
//
//struct GlobalConsts {
//  XMFLOAT4X4 WorldViewProj = Identity4x4();
//  XMFLOAT4X4 World = Identity4x4();
//  XMFLOAT4X4 RSInvT = Identity4x4();
//  XMFLOAT4 LightDir = {};
//  XMFLOAT4 EyePos = {};
//  XMFLOAT4X4 ShadowViewProj = Identity4x4();
//};
//
//struct BaseMaterialConsts {
//  XMFLOAT4 Ka = { 0.8f, 0.8f, 0.8f, 1.0f }; // Ambient Color Constant
//  XMFLOAT4 Kd = { 0.8f, 0.8f, 0.8f, 1.0f }; // Diffuse Color Constant
//  XMFLOAT4 Ks = { 0.5f, 0.5f, 0.5f, 1.0f }; // Specular Color Constant
//  float Ns = 323.999994f; // Shiniess
//  bool IsTextured = false; // Whether applied texture
//};
//
//void SetCubeGeo(BaseGeometry<Vertex>& geo);
//
//__declspec(align(16)) class MyApp : public BaseApp {
//public:
//  MyApp(HINSTANCE hInstance);
//  virtual ~MyApp();
//
//  void Start();
//  void Render();
//  void Update();
//
//  void RenderShadowMap();
//  void RenderObjects();
//  void RenderUI();
//  void MatrixBuild(XMVECTOR& viewPos, XMMATRIX& proj);
//
//  ComPtr<ID3D12DescriptorHeap> mDescHeap;
//  ComPtr<ID3D12DescriptorHeap> mUIDescHeap;
//
//  unique_ptr<BaseUploadHeap<GlobalConsts>> mConstBuffer;
//  unique_ptr<BaseUploadHeap<BaseMaterialConsts>> mMatBuffer;
//  unique_ptr<BaseImage> mTexture;
//
//  unique_ptr<BaseDrawContext> mDrawContext;
//  //unique_ptr<BaseDrawContext> mUIDrawContext;
//
//  XMVECTOR mLightDir;
//  XMVECTOR mViewPos;
//  XMVECTOR mSMPos;
//
//  unique_ptr<BaseCamera> mShadowMapCamera;
//  
//  // Shadow map depth buffer
//  ComPtr<ID3D12Resource> mShadowDepthStencilBuffer;
//  ComPtr<ID3D12DescriptorHeap> mShadowDsDescriptorHeap;
//
//  D3D12_CPU_DESCRIPTOR_HANDLE ShadowDepthBufferView() const;
//
//  void InitUI();
//  void InitShadowDepth();
//  void InitConstBuffer();
//  void InitTextureBuffer();
//  void InitMatBuffer();
//  void LoadObj();
//
//  void* operator new(size_t i)
//  {
//    return _mm_malloc(i, 16);
//  }
//
//  void operator delete(void* p)
//  {
//    _mm_free(p);
//  }
//
//  ImVec4 mClearColor;
//  
//};
