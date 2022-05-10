#pragma once

#include "BaseDescHeapHandle.h"


// refer to: https://alextardif.com/D3D11To12P2.html
class BaseRuntimeHeap
{
public:
	// Runtime heap includes two sections:
	// (1) Dynamic segment
	// (2) Static segement
	// Static segment is ordered after dynamic region
	BaseRuntimeHeap(ID3D12Device* device, UINT32 numDyncDescriptors=1024, UINT32 numStaticDescriptors = 128);

	void Reset();
	BaseDescHeapHandle GetHeapHandleBlock(UINT32 count);
	BaseDescHeapHandle GetHeapHandleStaticBlock();

	ComPtr<ID3D12DescriptorHeap> mDescHeap;

	D3D12_CPU_DESCRIPTOR_HANDLE mDescriptorHeapCPUStart;
	D3D12_GPU_DESCRIPTOR_HANDLE mDescriptorHeapGPUStart;

private:
	UINT32 mMaxDescDynamic; // Dynamic Descriptor Num (Reset after each update)
	UINT32 mMaxDescStatic; // Static Descriptor Num, for those static resources (UI font, etc.)
	UINT32 mDescriptorSize;
	UINT32 mCurrentDescriptorIndex;
	UINT32 mCurrentStaticDescriptorIndex;
};

