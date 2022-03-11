#pragma once

#include <windows.h>
#include "d3dx12.h"
#include <wrl.h>
#include "DebugOut.h"

using Microsoft::WRL::ComPtr;

template <class T>
class BaseUploadHeap
{
public:
  BaseUploadHeap(int num, ID3D12Device* device);
  ~BaseUploadHeap();

  void ConstructDescHeap(ID3D12Device* device, ID3D12DescriptorHeap* descHeap);
  void Upload(int idx);

  ComPtr<ID3D12Resource> mUploadHeap;
  T mData;
  UINT8* mCPUAddr;

  int mBlockSize;
  int mNum;

  UINT8* GetAddr(int idx);
};

template<class T>
UINT8* BaseUploadHeap<T>::GetAddr(int idx) {
  if (idx >= mNum) {
    dout::printf("UploadBuffer Index Overflow!\n");
    return nullptr;
  }
  return mCPUAddr + mBlockSize * idx;
}

template<class T>
BaseUploadHeap<T>::BaseUploadHeap(int num, ID3D12Device* device) : mNum(num)
{
  mBlockSize = (sizeof(T) + 255) & ~255;

  auto hprop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
  auto rdesc = CD3DX12_RESOURCE_DESC::Buffer(mBlockSize * mNum); // now we allocate a fixed large enough space
  HRESULT hr = device->CreateCommittedResource(
    &hprop, // this heap will be used to upload the constant buffer data
    D3D12_HEAP_FLAG_NONE, // no flags
    &rdesc, // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
    D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
    nullptr, // we do not have use an optimized clear value for constant buffers
    IID_PPV_ARGS(mUploadHeap.GetAddressOf())
  );

  CD3DX12_RANGE readRange(0, 0);
  hr = mUploadHeap->Map(0, &readRange, reinterpret_cast<void**>(&mCPUAddr));

  ZeroMemory(&mData, sizeof(mData));
  for (int i = 0; i < mNum; ++i)
  {
    memcpy(GetAddr(i), &mData, sizeof(mData));
  }
}

template<class T>
void BaseUploadHeap<T>::Upload(int idx)
{
  memcpy(GetAddr(i), &mData, sizeof(T));
}

template<class T>
void BaseUploadHeap<T>::ConstructDescHeap(ID3D12Device* device, ID3D12DescriptorHeap* descHeap)
{
  auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(descHeap->GetCPUDescriptorHandleForHeapStart());
  for (UINT i = 0; i < mNum; ++i)
  {
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = mUploadHeap->GetGPUVirtualAddress() + static_cast<D3D12_GPU_VIRTUAL_ADDRESS>(i) * mBlockSize;
    cbvDesc.SizeInBytes = mBlockSize;    // CB size is required to be 256-byte aligned.
    device->CreateConstantBufferView(&cbvDesc, handle);

    handle.Offset(1, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
  }
}

template<class T>
BaseUploadHeap<T>::~BaseUploadHeap()
{
  if (mUploadHeap != nullptr)
  {
    mUploadHeap->Unmap(0, nullptr);
  }
  mCPUAddr = nullptr;
}