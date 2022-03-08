#pragma once

#include <vector>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <windows.h>
#include "d3dx12.h"
#include <wrl.h>


using namespace DirectX;
using Microsoft::WRL::ComPtr;
using namespace std;

class GeometryView
{
public:
  D3D12_VERTEX_BUFFER_VIEW* mVertexBufferView;
  D3D12_INDEX_BUFFER_VIEW* mIndexBufferView;

  int numIndex;
};

template <class T>
class Geometry
{
public:
  vector<T> mVertices;
  vector<DWORD> mIndices;

  // Vertex Buffer
  ComPtr<ID3D12Resource> mVertexBuffer;
  D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
  ComPtr<ID3D12Resource> mvBufferUploadHeap;

  // Index Buffer
  ComPtr<ID3D12Resource> mIndexBuffer;
  D3D12_INDEX_BUFFER_VIEW mIndexBufferView;
  ComPtr<ID3D12Resource> miBufferUploadHeap;


  void UploadVertex(ComPtr<ID3D12Device>& device, ComPtr <ID3D12GraphicsCommandList>& commandList);
  void UploadIndex(ComPtr<ID3D12Device>& device, ComPtr <ID3D12GraphicsCommandList>& commandList);

  GeometryView GetGeoView();
};

template <class T>
GeometryView Geometry<T>::GetGeoView() {
  GeometryView ret;

  ret.mVertexBufferView = &mVertexBufferView;
  ret.mIndexBufferView = &mIndexBufferView;
  ret.numIndex = mIndices.size();

  return ret;
}

template <class T>
void Geometry<T>::UploadVertex(ComPtr<ID3D12Device>& device, ComPtr<ID3D12GraphicsCommandList>& commandList)
{
  int vBufferSize = sizeof(T) * mVertices.size();

  // create default heap
  // default heap is memory on the GPU. Only the GPU has access to this memory
  // To get data into this heap, we will have to upload the data using
  // an upload heap
  auto hprop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
  auto rdesc = CD3DX12_RESOURCE_DESC::Buffer(vBufferSize);
  device->CreateCommittedResource(
    &hprop, // a default heap
    D3D12_HEAP_FLAG_NONE, // no flags
    &rdesc, // resource description for a buffer
    D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
                                    // from the upload heap to this heap
    nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
    IID_PPV_ARGS(mVertexBuffer.GetAddressOf())
  );

  // we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
  //mVertexBuffer->SetName(L"Vertex Buffer Resource Heap");

  // create upload heap
  // upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
  // We will upload the vertex buffer using this heap to the default heap
  hprop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
  rdesc = CD3DX12_RESOURCE_DESC::Buffer(vBufferSize);
  device->CreateCommittedResource(
    &hprop,                                         // upload heap
    D3D12_HEAP_FLAG_NONE,                           // no flags
    &rdesc,                                         // resource description for a buffer
    D3D12_RESOURCE_STATE_GENERIC_READ,              // GPU will read from this buffer and copy its contents to the default heap
    nullptr,
    IID_PPV_ARGS(mvBufferUploadHeap.GetAddressOf())
  );
  //mvBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

  // store vertex buffer in upload heap
  D3D12_SUBRESOURCE_DATA vertexData = {};
  vertexData.pData = reinterpret_cast<BYTE*>(mVertices.data()); // pointer to our vertex array
  vertexData.RowPitch = vBufferSize; // size of all our triangle vertex data
  vertexData.SlicePitch = vBufferSize; // also the size of our triangle vertex data

  // we are now creating a command with the command list to copy the data from
  // the upload heap to the default heap
  UpdateSubresources(commandList.Get(), mVertexBuffer.Get(), mvBufferUploadHeap.Get(), 0, 0, 1, &vertexData);

  // transition the vertex buffer data from copy destination state to vertex buffer state
  auto trans = CD3DX12_RESOURCE_BARRIER::Transition(
    mVertexBuffer.Get(),
    D3D12_RESOURCE_STATE_COPY_DEST,
    D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
  );
  commandList->ResourceBarrier(1,
    &trans
  );

  // create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
  mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
  mVertexBufferView.StrideInBytes = sizeof(T);
  mVertexBufferView.SizeInBytes = vBufferSize;

}

template <class T>
void Geometry<T>::UploadIndex(ComPtr<ID3D12Device>& device, ComPtr<ID3D12GraphicsCommandList>& commandList)
{
  int iBufferSize = sizeof(T) * mIndices.size();

  // create default heap to hold index buffer
  auto hprop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
  auto bdesc = CD3DX12_RESOURCE_DESC::Buffer(iBufferSize);
  device->CreateCommittedResource(
    &hprop, // a default heap
    D3D12_HEAP_FLAG_NONE, // no flags
    &bdesc, // resource description for a buffer
    D3D12_RESOURCE_STATE_COPY_DEST, // start in the copy destination state
    nullptr, // optimized clear value must be null for this type of resource
    IID_PPV_ARGS(mIndexBuffer.GetAddressOf()));

  //// we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
  //vertexBuffer->SetName(L"Index Buffer Resource Heap");

  // create upload heap to upload index buffer
  hprop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
  bdesc = CD3DX12_RESOURCE_DESC::Buffer(iBufferSize);
  device->CreateCommittedResource(
    &hprop, // upload heap
    D3D12_HEAP_FLAG_NONE, // no flags
    &bdesc, // resource description for a buffer
    D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
    nullptr,
    IID_PPV_ARGS(miBufferUploadHeap.GetAddressOf())
  );
  //vBufferUploadHeap->SetName(L"Index Buffer Upload Resource Heap");

  // store vertex buffer in upload heap
  D3D12_SUBRESOURCE_DATA indexData = {};
  indexData.pData = reinterpret_cast<BYTE*>(mIndices.data()); // pointer to our index array
  indexData.RowPitch = iBufferSize; // size of all our index buffer
  indexData.SlicePitch = iBufferSize; // also the size of our index buffer

  // we are now creating a command with the command list to copy the data from
  // the upload heap to the default heap
  UpdateSubresources(commandList.Get(), mIndexBuffer.Get(), miBufferUploadHeap.Get(), 0, 0, 1, &indexData);

  // transition the vertex buffer data from copy destination state to vertex buffer state
  auto trans = CD3DX12_RESOURCE_BARRIER::Transition(
    mIndexBuffer.Get(), 
    D3D12_RESOURCE_STATE_COPY_DEST, 
    D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
  );
  commandList->ResourceBarrier(1, &trans);

  // create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
  mIndexBufferView.BufferLocation = mIndexBuffer->GetGPUVirtualAddress();
  mIndexBufferView.Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
  mIndexBufferView.SizeInBytes = iBufferSize;
}