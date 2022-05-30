#pragma once

#include <windows.h>
#include "../ThirdParty/d3dx12.h"
#include <wrl.h>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "BaseDescHeapHandle.h"

using namespace DirectX;

// refer to https ://stackoverflow.com/questions/67548981/how-to-render-to-texture-in-directx12-c-what-is-the-process

class BaseRenderTexture
{
public:
    BaseRenderTexture(DXGI_FORMAT format) noexcept;

    void SetDevice(_In_ ID3D12Device* device, BaseDescHeapHandle srvHandle, D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptor);

    void SizeResources(size_t width, size_t height);

    void ReleaseDevice() noexcept;

    void TransitionTo(_In_ ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES afterState);

    void BeginScene(_In_ ID3D12GraphicsCommandList* commandList)
    {
        TransitionTo(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }

    void EndScene(_In_ ID3D12GraphicsCommandList* commandList)
    {
        TransitionTo(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }

    void SetClearColor(FXMVECTOR color)
    {
        DirectX::XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(mClearColor), color);
    }

    ID3D12Resource* GetResource() const noexcept { return mResource.Get(); }
    D3D12_RESOURCE_STATES GetCurrentState() const noexcept { return mState; }

    void SetWindow(const RECT& rect);

    DXGI_FORMAT GetFormat() const noexcept { return mFormat; }

    D3D12_CPU_DESCRIPTOR_HANDLE                         mSrvDescriptor;
    D3D12_CPU_DESCRIPTOR_HANDLE                         mRtvDescriptor;

    BaseDescHeapHandle mSRVHandle;

    size_t                                              mWidth;
    size_t                                              mHeight;

    float                                               mClearColor[4];

private:
    Microsoft::WRL::ComPtr<ID3D12Device>                mDevice;
    Microsoft::WRL::ComPtr<ID3D12Resource>              mResource;
    D3D12_RESOURCE_STATES                               mState;

    DXGI_FORMAT                                         mFormat;
};

