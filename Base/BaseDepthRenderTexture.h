#pragma once
#include <windows.h>
#include "../ThirdParty/d3dx12.h"
#include <wrl.h>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "BaseDescHeapHandle.h"

using namespace DirectX;

class BaseDepthRenderTexture
{
public:
	BaseDepthRenderTexture(DXGI_FORMAT format);

	void SetDevice(_In_ ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE dsvDescriptor);
	void SizeResources(size_t width, size_t height);

	ComPtr<ID3D12Resource> mDepthStencilBuffer;
	D3D12_CPU_DESCRIPTOR_HANDLE mHandle;
};

