#pragma once
#include <windows.h>
#include "d3dx12.h"
#include <wrl.h>
#include <vector>
#include <dxgi1_4.h>
#include <unordered_map>
#include <memory>

using Microsoft::WRL::ComPtr;
using namespace std;

class BaseMainHeap
{
public:
  BaseMainHeap(ID3D12Device* device, int maxNumDesc=64);

  ComPtr<ID3D12DescriptorHeap> mDescHeap;
};

