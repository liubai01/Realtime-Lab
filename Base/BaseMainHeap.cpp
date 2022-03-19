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
}