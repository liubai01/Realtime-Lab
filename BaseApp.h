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

using namespace DirectX;
using namespace DirectX::PackedVector;

using Microsoft::WRL::ComPtr;
using namespace std;

struct ConstantBuffer {
  XMFLOAT4X4 WorldViewProj = Identity4x4();
};

class BaseApp
{
public:
  BaseApp(HINSTANCE hInstance);
  ~BaseApp();
  void Run();
  void Update();
  virtual void Render();
  void Flush();

  static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

  void ResetCommandList();
  void Init(HINSTANCE hInstance);

  void InitWindow(HINSTANCE hInstance);
  void InitDevice();
  void InitCommandQueue();
  void InitFence();
  void InitSwapChain();

  void InitRTV();
  void InitDepth();
  void InitConstBuffer();

  void InitRootSig();
  void InitInputLayout();
  void InitPSO();

  D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
  D3D12_CPU_DESCRIPTOR_HANDLE DepthBufferView() const;

  int mFrameCnt = 3;
  int mFrameIdx = 0;
  bool mIsRunning = true;
  vector<ComPtr<ID3D12Fence>> mFence;
  vector<UINT64> mExpectedFenceValue;

  ComPtr<ID3D12DescriptorHeap> mRtvDescriptorHeap;
  vector<ComPtr<ID3D12Resource>> mRenderTargets;

  int mWidth = 800;
  int mHeight = 600;
  HWND mHwnd;

  clock_t mTimer;
  float mTimeDelta;

  D3D12_VIEWPORT mViewport; // area that output from rasterizer will be stretched to.
  D3D12_RECT mScissorRect; // the area to draw in. pixels outside that area will not be drawn onto

  XMFLOAT4X4 mProj = Identity4x4();

  void InitView();

  vector<BaseRenderingObj> mObjs;
  template<typename V>
  BaseRenderingObj* RegisterGeo(BaseGeometry<V>& geo);

  vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
  D3D12_INPUT_LAYOUT_DESC mInputLayoutDesc;

  ComPtr<IDXGIFactory4> mDxgiFactory;
  ComPtr<IDXGISwapChain3> mSwapChain;

  HANDLE mFenceEvent;

  ComPtr<ID3D12Device> mDevice;

  ComPtr<ID3D12CommandQueue> mCommandQueue;
  ComPtr<ID3D12CommandAllocator> mCommandAlloc;
  ComPtr<ID3D12GraphicsCommandList> mCommandList;

  ComPtr<ID3D12PipelineState> mPSO;
  ComPtr<ID3D12RootSignature> mRootSig;

  ComPtr<ID3D12Resource> mDepthStencilBuffer;
  ComPtr<ID3D12DescriptorHeap> mDsDescriptorHeap;

  ShaderManager mShader;

#pragma region [Constant Buffer]

  ComPtr<ID3D12DescriptorHeap> mConstDescHeap;
  ComPtr<ID3D12Resource> mConstBufferUploadHeap;

  vector<D3D12_ROOT_PARAMETER> mRootParams;
  D3D12_DESCRIPTOR_RANGE mDescTableRanges[1];

  ConstantBuffer mCb;
  UINT8* mCbAddr;
  int mAlignSize;

#pragma endregion
};


template<typename V>
BaseRenderingObj* BaseApp::RegisterGeo(BaseGeometry<V>& geo) {
  ResetCommandList();

  mObjs.emplace_back();
  auto& obj = mObjs.back();

  obj.UploadGeo(geo, mDevice, mCommandList);

  // Now we execute the command list to upload the initial assets (triangle data)
  mCommandList->Close();
  ID3D12CommandList* ppCommandLists[] = { mCommandList.Get() };
  mCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

  // increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
  // because we only have one commitlist ( sadly :( )
  ++mExpectedFenceValue[mFrameIdx];
  HRESULT hr = mCommandQueue->Signal(mFence[mFrameIdx].Get(), mExpectedFenceValue[mFrameIdx]);
  if (FAILED(hr))
  {
    mIsRunning = false;
  }

  UINT64 nowFenceValue = mFence[mFrameIdx]->GetCompletedValue();
  if (nowFenceValue < mExpectedFenceValue[mFrameIdx])
  {
    hr = mFence[mFrameIdx]->SetEventOnCompletion(mExpectedFenceValue[mFrameIdx], mFenceEvent);
    if (FAILED(hr))
    {
      mIsRunning = false;
      return nullptr;
    }

    WaitForSingleObject(mFenceEvent, INFINITE);
  }
  
  return &obj;
}