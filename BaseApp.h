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
#include "Geometry.h"
#include <time.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

using Microsoft::WRL::ComPtr;
using namespace std;

struct ConstantBuffer {
  XMFLOAT4 colorMultiplier;
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

  int mWidth = 800;
  int mHeight = 600;
  HWND mHwnd;

  clock_t mTimer;
  float mTimeDelta;

  D3D12_VIEWPORT mViewport; // area that output from rasterizer will be stretched to.
  D3D12_RECT mScissorRect; // the area to draw in. pixels outside that area will not be drawn onto
  void InitView();

  //vector<D3D12_VERTEX_BUFFER_VIEW> mVBuffViews;
  //vector<D3D12_INDEX_BUFFER_VIEW> mIBuffViews;
  vector<GeometryView> mGeos;

  template<typename V>
  void RegisterGeo(Geometry<V>& geo);

  vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
  D3D12_INPUT_LAYOUT_DESC mInputLayoutDesc;

  ComPtr<IDXGIFactory4> mDxgiFactory;
  ComPtr<IDXGISwapChain3> mSwapChain;

  HANDLE mFenceEvent;

  ComPtr<ID3D12Device> mDevice;

  ComPtr<ID3D12CommandQueue> mCommandQueue;
  ComPtr<ID3D12CommandAllocator> mCommandAlloc;
  ComPtr<ID3D12GraphicsCommandList> mCommandList;

  vector<ComPtr<ID3D12Fence>> mFence;
  vector<UINT64> mExpectedFenceValue;

  ComPtr<ID3D12DescriptorHeap> mRtvDescriptorHeap;
  vector<ComPtr<ID3D12Resource>> mRenderTargets;

  ComPtr<ID3D12PipelineState> mPSO;
  ComPtr<ID3D12RootSignature> mRootSig;

  ComPtr<ID3D12Resource> mDepthStencilBuffer;
  ComPtr<ID3D12DescriptorHeap> mDsDescriptorHeap;

  ShaderManager mShader;

  ComPtr<ID3D12DescriptorHeap> mConstDescHeap;
  ComPtr<ID3D12Resource> mConstBufferUploadHeap;

  vector<D3D12_ROOT_PARAMETER> mRootParams;
  D3D12_DESCRIPTOR_RANGE mDescTableRanges[1];

  ConstantBuffer mCbColorMultiplierData;
  UINT8* mCbColorMultiplierAddr;
};


template<typename V>
void BaseApp::RegisterGeo(Geometry<V>& geo) {
  ResetCommandList();

  geo.UploadVertex(mDevice, mCommandList);
  geo.UploadIndex(mDevice, mCommandList);

  //mVBuffViews.push_back(geo.mVertexBufferView);
  //mIBuffViews.push_back(geo.mIndexBufferView);
  mGeos.push_back(geo.GetGeoView());

  // Now we execute the command list to upload the initial assets (triangle data)
  mCommandList->Close();
  ID3D12CommandList* ppCommandLists[] = { mCommandList.Get() };
  mCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

  // increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
  // but we only have one commitlist ( sadly :( )
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
      return;
    }

    WaitForSingleObject(mFenceEvent, INFINITE);
  }
  
}