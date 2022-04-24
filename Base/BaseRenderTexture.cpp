#include "BaseRenderTexture.h"
#include "../DebugOut.h"

// refer to https://stackoverflow.com/questions/67548981/how-to-render-to-texture-in-directx12-c-what-is-the-process

BaseRenderTexture::BaseRenderTexture(DXGI_FORMAT format) noexcept :
    m_state(D3D12_RESOURCE_STATE_COMMON),
    m_srvDescriptor{},
    m_rtvDescriptor{},
    m_clearColor{},
    m_format(format),
    m_width(0),
    m_height(0)
{
}

void BaseRenderTexture::SetDevice(_In_ ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE srvDescriptor, D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptor)
{
    if (device == m_device.Get()
        && srvDescriptor.ptr == m_srvDescriptor.ptr
        && rtvDescriptor.ptr == m_rtvDescriptor.ptr)
        return;

    {
        D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = { m_format, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
        if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
        {
            throw std::runtime_error("CheckFeatureSupport");
        }

        UINT required = D3D12_FORMAT_SUPPORT1_TEXTURE2D | D3D12_FORMAT_SUPPORT1_RENDER_TARGET;
        if ((formatSupport.Support1 & required) != required)
        {
#ifdef _DEBUG
            char buff[128] = {};
            sprintf_s(buff, "RenderTexture: Device does not support the requested format (%u)!\n", m_format);
            OutputDebugStringA(buff);
#endif
            throw std::runtime_error("RenderTexture");
        }
    }

    if (!srvDescriptor.ptr || !rtvDescriptor.ptr)
    {
        throw std::runtime_error("Invalid descriptors");
    }

    m_device = device;

    m_srvDescriptor = srvDescriptor;
    m_rtvDescriptor = rtvDescriptor;
}

void BaseRenderTexture::SizeResources(size_t width, size_t height)
{
    if (width == m_width && height == m_height)
        return;

    if (m_width > UINT32_MAX || m_height > UINT32_MAX)
    {
        throw std::out_of_range("Invalid width/height");
    }

    if (!m_device)
        return;

    m_width = m_height = 0;

    auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(m_format,
        static_cast<UINT64>(width),
        static_cast<UINT>(height),
        1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

    D3D12_CLEAR_VALUE clearValue = { m_format, {} };
    memcpy(clearValue.Color, m_clearColor, sizeof(clearValue.Color));

    m_state = D3D12_RESOURCE_STATE_RENDER_TARGET;

    // Create a render target
    ThrowIfFailed(
        m_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
            &desc,
            m_state, &clearValue,
            IID_PPV_ARGS(m_resource.ReleaseAndGetAddressOf()))
    );

    // Create RTV.
    m_device->CreateRenderTargetView(m_resource.Get(), nullptr, m_rtvDescriptor);

    // Create SRV.
    m_device->CreateShaderResourceView(m_resource.Get(), nullptr, m_srvDescriptor);

    m_width = width;
    m_height = height;
}

void BaseRenderTexture::ReleaseDevice() noexcept
{
    m_resource.Reset();
    m_device.Reset();

    m_state = D3D12_RESOURCE_STATE_COMMON;
    m_width = m_height = 0;

    m_srvDescriptor.ptr = m_rtvDescriptor.ptr = 0;
}

void BaseRenderTexture::TransitionTo(_In_ ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES afterState)
{
    if (m_state == afterState) {
        return;
    }
    auto trans = CD3DX12_RESOURCE_BARRIER::Transition(
        m_resource.Get(),
        m_state,
        afterState
    );
    commandList->ResourceBarrier(
        1,
        &trans
    );
    m_state = afterState;
}

void BaseRenderTexture::SetWindow(const RECT& output)
{
    // Determine the render target size in pixels.
    auto width = size_t(std::max<LONG>(output.right - output.left, 1));
    auto height = size_t(std::max<LONG>(output.bottom - output.top, 1));

    SizeResources(width, height);
}