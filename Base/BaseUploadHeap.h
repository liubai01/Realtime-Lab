#pragma once

#include <windows.h>
#include "../ThirdParty/d3dx12.h"
#include <wrl.h>
#include "../DebugOut.h"
#include "BaseMainHeap.h"

using Microsoft::WRL::ComPtr;

template <class T>
class BaseUploadHeap
{
public:
  BaseUploadHeap(ID3D12Device* device);
  ~BaseUploadHeap();

  void RegisiterHeap(BaseMainHeap* mainHeap);
  void Upload();

  ComPtr<ID3D12Resource> mUploadHeap;
  T mData;

  BaseDescHeapHandle* GetHandle();

private: 
  UINT8* mCPUAddr;
  int mBlockSize;
  BaseDescHeapHandle* mHandle;
  BaseMainHeap* mMainHeap;

  ID3D12Device* mDevice;
};

template<class T>
BaseUploadHeap<T>::BaseUploadHeap(ID3D12Device* device)
{
  mBlockSize = (sizeof(T) + 255) & ~255;
  auto hprop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
  auto rdesc = CD3DX12_RESOURCE_DESC::Buffer(static_cast<UINT64>(mBlockSize));
  HRESULT hr = device->CreateCommittedResource(
    &hprop, // this heap will be used to upload the constant buffer data
    D3D12_HEAP_FLAG_NONE, // no flags
    &rdesc, // size of the resource heap. Must be a multiple of 256 bytes for single-textures and constant buffers
    D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
    nullptr, // we do not have use an optimized clear value for constant buffers
    IID_PPV_ARGS(mUploadHeap.GetAddressOf())
  );

  CD3DX12_RANGE readRange(0, 0);
  hr = mUploadHeap->Map(0, &readRange, reinterpret_cast<void**>(&mCPUAddr));

  ZeroMemory(&mData, sizeof(mData));
  memcpy(mCPUAddr, &mData, sizeof(mData));

  mHandle = nullptr;
  mDevice = device;
}

template<class T>
void BaseUploadHeap<T>::Upload()
{
  memcpy(mCPUAddr, &mData, sizeof(T));
}

template<class T>
void BaseUploadHeap<T>::RegisiterHeap(BaseMainHeap* mainHeap)
{
  mHandle = new BaseDescHeapHandle(mainHeap->GetNewHeapHandle());

  D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
  cbvDesc.BufferLocation = mUploadHeap->GetGPUVirtualAddress();
  cbvDesc.SizeInBytes = mBlockSize;    // CB size is required to be 256-byte aligned.
  mDevice->CreateConstantBufferView(&cbvDesc, mHandle->GetCPUHandle());

  mMainHeap = mainHeap;
}

template<class T>
BaseDescHeapHandle* BaseUploadHeap<T>::GetHandle()
{
  if (mHandle == nullptr)
  {
    dout::printf("[BaseUploadHeap] The buffer has not been registered in heap.");
  }
  return mHandle;
}

//template<class T>
//void BaseUploadHeap<T>::AppendDesc(ID3D12Device* device, ID3D12DescriptorHeap* descHeap, int offset)
//{
//  auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(descHeap->GetCPUDescriptorHandleForHeapStart());
//  handle.Offset(offset, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
//  for (UINT i = 0; i < static_cast<UINT>(mNum); ++i)
//  {
//    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
//    cbvDesc.BufferLocation = mUploadHeap->GetGPUVirtualAddress() + static_cast<D3D12_GPU_VIRTUAL_ADDRESS>(i) * mBlockSize;
//    cbvDesc.SizeInBytes = mBlockSize;    // CB size is required to be 256-byte aligned.
//    device->CreateConstantBufferView(&cbvDesc, handle);
//
//    handle.Offset(1, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
//  }
//}

template<class T>
BaseUploadHeap<T>::~BaseUploadHeap()
{
  if (mUploadHeap != nullptr)
  {
    mUploadHeap->Unmap(0, nullptr);
  }
  mCPUAddr = nullptr;
  if (mHandle)
  {
    mMainHeap->FreeHeapHandle(*mHandle);
    delete mHandle;
  }
}