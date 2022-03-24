#include "BaseRuntimeHeap.h"
#include "../DebugOut.h"

BaseRuntimeHeap::BaseRuntimeHeap(ID3D12Device* device, UINT32 numDescriptors)
{
  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
  heapDesc.NumDescriptors = numDescriptors;
  heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

  HRESULT hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mDescHeap.GetAddressOf()));
  ThrowIfFailed(hr);

  mCurrentDescriptorIndex = 0;

  mMaxDesc = numDescriptors;
  mDescriptorHeapCPUStart = mDescHeap->GetCPUDescriptorHandleForHeapStart();
  mDescriptorHeapGPUStart = mDescHeap->GetGPUDescriptorHandleForHeapStart();
  mDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

BaseDescHeapHandle BaseRuntimeHeap::GetHeapHandleBlock(UINT32 count)
{
  UINT32 newHandleID = 0;
  UINT32 blockEnd = mCurrentDescriptorIndex + count;

  if (blockEnd < mMaxDesc)
  {
    newHandleID = mCurrentDescriptorIndex;
    mCurrentDescriptorIndex = blockEnd;
  }
  else
  {
    dout::printf("Ran out of render pass descriptor heap handles, need to increase heap size.\n");
  }

  BaseDescHeapHandle newHandle;
  D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = mDescriptorHeapCPUStart;
  cpuHandle.ptr += newHandleID * mDescriptorSize;
  newHandle.SetCPUHandle(cpuHandle);

  D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = mDescriptorHeapGPUStart;
  gpuHandle.ptr += static_cast<UINT64>(newHandleID) * mDescriptorSize;
  newHandle.SetGPUHandle(gpuHandle);

  newHandle.SetHeapIndex(newHandleID);

  return newHandle;
}

void BaseRuntimeHeap::Reset()
{
  mCurrentDescriptorIndex = 0;
}
