#pragma once
#include <windows.h>
#include "../ThirdParty/d3dx12.h"
#include <wrl.h>
#include <vector>
#include <dxgi1_4.h>
#include <unordered_map>
#include <queue>
#include <memory>

#include "BaseDescHeapHandle.h"

using Microsoft::WRL::ComPtr;

// refer to: https://alextardif.com/D3D11To12P2.html

class BaseMainHeap
{
public:
	BaseMainHeap(ID3D12Device* device, int maxNumDesc=64);
	~BaseMainHeap();

	BaseDescHeapHandle GetNewHeapHandle();
	void FreeHeapHandle(BaseDescHeapHandle handle);

	ComPtr<ID3D12DescriptorHeap> mDescHeap;
private:
	std::queue<UINT32> mFreeDescriptors;
	UINT32 mCurrentDescriptorIndex;
	UINT32 mActiveHandleCount;

	UINT32 mMaxDesc;
	D3D12_CPU_DESCRIPTOR_HANDLE mDescriptorHeapCPUStart;
	D3D12_GPU_DESCRIPTOR_HANDLE mDescriptorHeapGPUStart;
	UINT32 mDescriptorSize;
};

