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
#include "BaseShaderManager.h"
#include "../DebugOut.h"
#include "BaseGeometry.h"
#include "BaseObject.h"
#include "../MathUtils.h"
#include <time.h>
#include "BaseDrawContext.h"
#include "BaseCamera.h"
#include "BaseRuntimeHeap.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

using Microsoft::WRL::ComPtr;
using namespace std;

class BaseApp
{
public:
  BaseApp(HINSTANCE hInstance);
  virtual ~BaseApp();
  void Run();
  virtual void Update() = 0;
  virtual void Render() = 0;
  virtual void Start() {};

  void Enqueue(ID3D12GraphicsCommandList* commandList);
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

  int mWidth = 1440;
  int mHeight = 900;
  HWND mHwnd;

  clock_t mTimer;
  float mTimeDelta;

  unordered_map<string, shared_ptr<BaseObject>>* mObjs;

  ComPtr<IDXGIFactory4> mDxgiFactory;
  ComPtr<IDXGISwapChain3> mSwapChain;

  ComPtr<ID3D12Device> mDevice;

  ComPtr<ID3D12CommandQueue> mCommandQueue;

  ComPtr<ID3D12Resource> mDepthStencilBuffer;
  ComPtr<ID3D12DescriptorHeap> mDsDescriptorHeap;

  BaseCamera* mMainCamera;

  BaseMainHeap* mMainHeap;
  BaseRuntimeHeap* mRuntimeHeap;

  shared_ptr<BaseObject> CreateObject(const string& name);
  shared_ptr<BaseObject> GetObject(const string& name);
};