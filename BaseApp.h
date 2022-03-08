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
  void InitRootSig();

  void InitInputLayout();

  void InitPSO();

  D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;

  int mFrameCnt = 3;
  int mFrameIdx = 0;
  bool mIsRunning = true;

  int mWidth = 800;
  int mHeight = 600;
  HWND mHwnd;

  D3D12_VIEWPORT mViewport; // area that output from rasterizer will be stretched to.
  D3D12_RECT mScissorRect; // the area to draw in. pixels outside that area will not be drawn onto
  void InitView();

  vector<D3D12_VERTEX_BUFFER_VIEW> mVBuffViews;

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

  ShaderManager mShader;

};


template<typename V>
void BaseApp::RegisterGeo(Geometry<V>& geo) {
  ResetCommandList();
  int vBufferSize = sizeof(geo.mVertices[0]) * geo.mVertices.size();

  // create default heap
  // default heap is memory on the GPU. Only the GPU has access to this memory
  // To get data into this heap, we will have to upload the data using
  // an upload heap
  auto hprop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
  auto rdesc = CD3DX12_RESOURCE_DESC::Buffer(vBufferSize);
  mDevice->CreateCommittedResource(
    &hprop, // a default heap
    D3D12_HEAP_FLAG_NONE, // no flags
    &rdesc, // resource description for a buffer
    D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
                                    // from the upload heap to this heap
    nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
    IID_PPV_ARGS(geo.mVertexBuffer.GetAddressOf())
  );

  // we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
  //mVertexBuffer->SetName(L"Vertex Buffer Resource Heap");

  // create upload heap
  // upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
  // We will upload the vertex buffer using this heap to the default heap
  hprop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
  rdesc = CD3DX12_RESOURCE_DESC::Buffer(vBufferSize);
  mDevice->CreateCommittedResource(
    &hprop,                                         // upload heap
    D3D12_HEAP_FLAG_NONE,                           // no flags
    &rdesc,                                         // resource description for a buffer
    D3D12_RESOURCE_STATE_GENERIC_READ,              // GPU will read from this buffer and copy its contents to the default heap
    nullptr,
    IID_PPV_ARGS(geo.mvBufferUploadHeap.GetAddressOf())
  );
  //mvBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

  // store vertex buffer in upload heap
  geo.mVertexData = {};
  geo.mVertexData.pData = reinterpret_cast<BYTE*>(geo.mVertices.data()); // pointer to our vertex array
  geo.mVertexData.RowPitch = vBufferSize; // size of all our triangle vertex data
  geo.mVertexData.SlicePitch = vBufferSize; // also the size of our triangle vertex data

  // we are now creating a command with the command list to copy the data from
  // the upload heap to the default heap
  UpdateSubresources(mCommandList.Get(), geo.mVertexBuffer.Get(), geo.mvBufferUploadHeap.Get(), 0, 0, 1, &geo.mVertexData);

  // transition the vertex buffer data from copy destination state to vertex buffer state
  auto trans = CD3DX12_RESOURCE_BARRIER::Transition(
    geo.mVertexBuffer.Get(),
    D3D12_RESOURCE_STATE_COPY_DEST,
    D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
  );
  mCommandList->ResourceBarrier(1,
    &trans
  );

  // Now we execute the command list to upload the initial assets (triangle data)
  mCommandList->Close();
  ID3D12CommandList* ppCommandLists[] = { mCommandList.Get() };
  mCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

  // increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
  ++mExpectedFenceValue[mFrameIdx];
  HRESULT hr = mCommandQueue->Signal(mFence[mFrameIdx].Get(), mExpectedFenceValue[mFrameIdx]);
  if (FAILED(hr))
  {
    mIsRunning = false;
  }

  // create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
  geo.mVertexBufferView.BufferLocation = geo.mVertexBuffer->GetGPUVirtualAddress();
  geo.mVertexBufferView.StrideInBytes = sizeof(V);
  geo.mVertexBufferView.SizeInBytes = vBufferSize;

  mVBuffViews.push_back(geo.mVertexBufferView);

}