#pragma once

#include "../ThirdParty/d3dx12.h"
#include "BaseUploadHeap.h"
#include "BaseMainHeap.h"
#include "BaseRuntimeHeap.h"
#include <wrl.h>

template <class T>
class BaseStagedBuffer
{
public:
  BaseStagedBuffer(ID3D12Device* device);

  void RegisterMainHandle(BaseMainHeap* heap);
  void RegisterRuntimeHandle(BaseRuntimeHeap* heap);

  bool mRuntimeRegistered;
  // would be invoked when GetHandle()
  virtual void Upload() = 0;
  BaseDescHeapHandle GetHandle();


  BaseUploadHeap<T> mBuffer;
private:
  BaseDescHeapHandle mRuntimeHandle;
  ID3D12Device* mDevice;
};

template <class T>
BaseStagedBuffer<T>::BaseStagedBuffer(ID3D12Device* device) : mBuffer(device)
{
  mDevice = device;
  mRuntimeRegistered = false;
}

template <class T>
void BaseStagedBuffer<T>::RegisterMainHandle(BaseMainHeap* heap)
{
  mBuffer.RegisiterHeap(heap);
}

template <class T>
void BaseStagedBuffer<T>::RegisterRuntimeHandle(BaseRuntimeHeap* heap)
{
  if (!mBuffer.GetHandle())
  {
    dout::printf("[BaseStagedBuffer] Could not register runtime heap handle before on main heap.");
  }

  mRuntimeHandle = heap->GetHeapHandleBlock(1);
  mDevice->CopyDescriptorsSimple(
    1,
    mRuntimeHandle.GetCPUHandle(),
    mBuffer.GetHandle()->GetCPUHandle(),
    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
  );
  mRuntimeRegistered = true;
}

template <class T>
BaseDescHeapHandle BaseStagedBuffer<T>::GetHandle()
{
  if (!mRuntimeRegistered)
  {
    dout::printf("[BaseStagedBuffer] The runtime handle has not been registered.");
  }

  // Derived class should copy their data to mBuffer.mData in Upload()
  Upload();

  // mBuffer copy data to CPU
  mBuffer.Upload();

  return mRuntimeHandle;
}