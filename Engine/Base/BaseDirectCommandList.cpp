#include "BaseDirectCommandList.h"
#include "../DebugOut.h"

BaseDirectCommandList::BaseDirectCommandList(ID3D12Device* device)
{
  HRESULT hr = device->CreateCommandAllocator(
    D3D12_COMMAND_LIST_TYPE_DIRECT,
    IID_PPV_ARGS(mCommandAlloc.GetAddressOf())
  );

  ThrowIfFailed(hr);

  hr = device->CreateCommandList(
    0,
    D3D12_COMMAND_LIST_TYPE_DIRECT,
    mCommandAlloc.Get(),
    nullptr,
    IID_PPV_ARGS(mCommandList.GetAddressOf())
  );
  ThrowIfFailed(hr);
  mCommandList->Close();
}

void BaseDirectCommandList::ResetCommandList()
{
  // Reuse the memory associated with command recording.
  // We can only reset when the associated command lists have finished execution on the GPU.
  HRESULT hr = mCommandAlloc->Reset();
  if (FAILED(hr))
  {
    dout::printf("[BaseDirectCommandList] Command Allocated Failed!");
  }
  ThrowIfFailed(hr);

  // A command list can be reset after it has been added to the command queue via ExecuteCommandList.
  // Reusing the command list reuses memory.
  hr = mCommandList->Reset(mCommandAlloc.Get(), nullptr);
  if (FAILED(hr))
  {
    dout::printf("[BaseDirectCommandList] Command List Failed!");
  }
  ThrowIfFailed(hr);
}