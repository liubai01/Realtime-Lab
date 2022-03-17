#pragma once

#include <windows.h>
#include "d3dx12.h"
#include <wrl.h>
#include <vector>
#include <dxgi1_4.h>
#include <unordered_map>
#include <memory>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "ShaderManager.h"
#include "DebugOut.h"
#include "BaseGeometry.h"
#include "BaseRenderingObj.h"
#include "MathUtils.h"
#include <time.h>
#include "BaseDrawContext.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

using Microsoft::WRL::ComPtr;
using namespace std;

class BaseApp
{
public:
  BaseApp(HINSTANCE hInstance);
  ~BaseApp();
  void Run();
  virtual void Update() = 0;
  virtual void Render() = 0;
  virtual void Start() {};

  void Flush(ID3D12GraphicsCommandList* commandList);
  void Swap();

  static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

  void InitImGUI();
  void InitWindow(HINSTANCE hInstance);
  void InitDevice();
  void InitCommandQueue();
  void InitFence();
  void InitSwapChain();

  void InitRTV();
  void InitDepth();

  D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
  D3D12_CPU_DESCRIPTOR_HANDLE DepthBufferView() const;

  int mFrameCnt = 3;
  int mFrameIdx = 0;
  bool mIsRunning = true;
  vector<ComPtr<ID3D12Fence>> mFence;
  vector<UINT64> mExpectedFenceValue;
  HANDLE mFenceEvent;

  ComPtr<ID3D12DescriptorHeap> mRtvDescriptorHeap;
  vector<ComPtr<ID3D12Resource>> mRenderTargets;

  int mWidth = 1024;
  int mHeight = 768;
  HWND mHwnd;

  clock_t mTimer;
  float mTimeDelta;

  vector<BaseRenderingObj> mObjs;
  template<typename V>
  BaseRenderingObj* RegisterGeo(BaseGeometry<V>& geo, unique_ptr<BaseDrawContext>& drawContext);

  ComPtr<IDXGIFactory4> mDxgiFactory;
  ComPtr<IDXGISwapChain3> mSwapChain;

  ComPtr<ID3D12Device> mDevice;

  ComPtr<ID3D12CommandQueue> mCommandQueue;

  ComPtr<ID3D12Resource> mDepthStencilBuffer;
  ComPtr<ID3D12DescriptorHeap> mDsDescriptorHeap;

  D3D12_VIEWPORT mViewport; // area that output from rasterizer will be stretched to.
  D3D12_RECT mScissorRect; // the area to draw in. pixels outside that area will not be drawn onto
  XMFLOAT4X4 mProj = Identity4x4();
  void InitView();
};


template<typename V>
BaseRenderingObj* BaseApp::RegisterGeo(BaseGeometry<V>& geo, unique_ptr<BaseDrawContext>& drawContext) {
  mObjs.emplace_back();
  auto& obj = mObjs.back();

  obj.UploadGeo(geo, mDevice.Get(), drawContext->mCommandList.Get());

  return &obj;
}