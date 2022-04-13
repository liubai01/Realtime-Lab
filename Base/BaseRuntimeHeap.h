#pragma once

#include "BaseDescHeapHandle.h"


// refer to: https://alextardif.com/D3D11To12P2.html
class BaseRuntimeHeap
{
public:
  BaseRuntimeHeap(ID3D12Device* device, UINT32 numDescriptors=64);

  void Reset();
  BaseDescHeapHandle GetHeapHandleBlock(UINT32 count);

  ComPtr<ID3D12DescriptorHeap> mDescHeap;

  D3D12_CPU_DESCRIPTOR_HANDLE mDescriptorHeapCPUStart;
  D3D12_GPU_DESCRIPTOR_HANDLE mDescriptorHeapGPUStart;

private:
  UINT32 mMaxDesc;
  UINT32 mDescriptorSize;
  UINT32 mCurrentDescriptorIndex;
};

