#pragma once
#include <windows.h>
#include "../ThirdParty/d3dx12.h"
#include <wrl.h>
#include <dxgi1_4.h>

using Microsoft::WRL::ComPtr;
using namespace std;

// refer to: https://alextardif.com/D3D11To12P2.html
class BaseDescHeapHandle
{
public:
  BaseDescHeapHandle()
  {
    mCPUHandle.ptr = NULL;
    mGPUHandle.ptr = NULL;
    mHeapIndex = 0;
  }

  D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return mCPUHandle; }
  D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return mGPUHandle; }
  UINT32 GetHeapIndex() { return mHeapIndex; }

  void SetCPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) { mCPUHandle = cpuHandle; }
  void SetGPUHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) { mGPUHandle = gpuHandle; }
  void SetHeapIndex(UINT32 heapIndex) { mHeapIndex = heapIndex; }

  bool IsValid() { return mCPUHandle.ptr != NULL; }
  bool IsReferencedByShader() { return mGPUHandle.ptr != NULL; }

private:
  D3D12_CPU_DESCRIPTOR_HANDLE mCPUHandle;
  D3D12_GPU_DESCRIPTOR_HANDLE mGPUHandle;
  UINT32 mHeapIndex;
};