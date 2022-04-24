#pragma once

#include <windows.h>
#include "../ThirdParty/d3dx12.h"
#include <wrl.h>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;

// refer to https ://stackoverflow.com/questions/67548981/how-to-render-to-texture-in-directx12-c-what-is-the-process

class BaseRenderTexture
{
public:
    BaseRenderTexture(DXGI_FORMAT format) noexcept;

    void SetDevice(_In_ ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE srvDescriptor, D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptor);

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
        DirectX::XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(m_clearColor), color);
    }

    ID3D12Resource* GetResource() const noexcept { return m_resource.Get(); }
    D3D12_RESOURCE_STATES GetCurrentState() const noexcept { return m_state; }

    void SetWindow(const RECT& rect);

    DXGI_FORMAT GetFormat() const noexcept { return m_format; }

    D3D12_CPU_DESCRIPTOR_HANDLE                         m_srvDescriptor;
    D3D12_CPU_DESCRIPTOR_HANDLE                         m_rtvDescriptor;

    size_t                                              m_width;
    size_t                                              m_height;

private:
    Microsoft::WRL::ComPtr<ID3D12Device>                m_device;
    Microsoft::WRL::ComPtr<ID3D12Resource>              m_resource;
    D3D12_RESOURCE_STATES                               m_state;
    float                                               m_clearColor[4];

    DXGI_FORMAT                                         m_format;
};

