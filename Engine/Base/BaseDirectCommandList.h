#pragma once
#include <windows.h>
#include "../ThirdParty/d3dx12.h"
#include <wrl.h>
#include <dxgi1_4.h>
#include <unordered_map>
#include <memory>

using Microsoft::WRL::ComPtr;

class BaseDirectCommandList
{
public:
  BaseDirectCommandList(ID3D12Device* device);

  ComPtr<ID3D12CommandAllocator> mCommandAlloc;
  ComPtr<ID3D12GraphicsCommandList> mCommandList;

  void ResetCommandList();

};

