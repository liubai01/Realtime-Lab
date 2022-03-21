#include "BaseMainHeap.h"
#include "../DebugOut.h"

BaseMainHeap::BaseMainHeap(ID3D12Device* device, int maxNumDesc)
{
  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
  heapDesc.NumDescriptors = maxNumDesc;
  heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

  HRESULT hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mDescHeap.GetAddressOf()));
  ThrowIfFailed(hr);

  mCurrentDescriptorIndex = 0;
  mActiveHandleCount = 0;

  mMaxDesc = maxNumDesc;
  mDescriptorHeapCPUStart = mDescHeap->GetCPUDescriptorHandleForHeapStart();
  mDescriptorHeapGPUStart = mDescHeap->GetGPUDescriptorHandleForHeapStart();
  mDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

}

BaseMainHeap::~BaseMainHeap()
{
  if (mActiveHandleCount != 0)
  {
    dout::printf("[BaseMainHeap] Descriptor heap is free when there is still handle inside.");
  }
}

BaseDescHeapHandle BaseMainHeap::GetNewHeapHandle()
{
  UINT32 newHandleID = 0;

  if (mCurrentDescriptorIndex < mMaxDesc)
  {
    newHandleID = mCurrentDescriptorIndex;
    mCurrentDescriptorIndex++;
  }
  else if (mFreeDescriptors.size() > 0)
  {
    newHandleID = mFreeDescriptors.front();
    mFreeDescriptors.pop();
  }
  else
  {
    dout::printf("[BaseMainHeap] Ran out of dynamic descriptor heap handles, need to increase heap size.");
  }

  BaseDescHeapHandle newHandle;
  D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = mDescriptorHeapCPUStart;
  cpuHandle.ptr += newHandleID * mDescriptorSize;
  newHandle.SetCPUHandle(cpuHandle);
  newHandle.SetHeapIndex(newHandleID);
  mActiveHandleCount++;

  return newHandle;
}

void BaseMainHeap::FreeHeapHandle(BaseDescHeapHandle handle)
{
  mFreeDescriptors.push(handle.GetHeapIndex());

  if (mActiveHandleCount == 0)
  {
    dout::printf("[BaseMainHeap] Freeing heap handles when there should be none left");
  }
  mActiveHandleCount--;
}