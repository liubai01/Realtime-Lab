#include "BaseRenderTexture.h"
#include "../DebugOut.h"

// refer to https://stackoverflow.com/questions/67548981/how-to-render-to-texture-in-directx12-c-what-is-the-process

BaseRenderTexture::BaseRenderTexture(DXGI_FORMAT format) noexcept :
    mState(D3D12_RESOURCE_STATE_COMMON),
    mSrvDescriptor{},
    mRtvDescriptor{},
    mClearColor{},
    mFormat(format),
    mWidth(0),
    mHeight(0)
{
}

void BaseRenderTexture::SetDevice(_In_ ID3D12Device* device, BaseDescHeapHandle srvHandle, D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptor)
{
    D3D12_CPU_DESCRIPTOR_HANDLE srvDescriptor = srvHandle.GetCPUHandle();
    if (device == mDevice.Get()
        && srvDescriptor.ptr == mSrvDescriptor.ptr
        && rtvDescriptor.ptr == mRtvDescriptor.ptr)
        return;

    {
        D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = { mFormat, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
        if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
        {
            throw std::runtime_error("CheckFeatureSupport");
        }

        UINT required = D3D12_FORMAT_SUPPORT1_TEXTURE2D | D3D12_FORMAT_SUPPORT1_RENDER_TARGET;
        if ((formatSupport.Support1 & required) != required)
        {
#ifdef _DEBUG
            char buff[128] = {};
            sprintf_s(buff, "RenderTexture: Device does not support the requested format (%u)!\n", mFormat);
            OutputDebugStringA(buff);
#endif
            throw std::runtime_error("RenderTexture");
        }
    }

    if (!srvDescriptor.ptr || !rtvDescriptor.ptr)
    {
        throw std::runtime_error("Invalid descriptors");
    }

    mDevice = device;

    mSrvDescriptor = srvDescriptor;
    mRtvDescriptor = rtvDescriptor;
    mSRVHandle = srvHandle;
}

void BaseRenderTexture::SizeResources(size_t width, size_t height)
{
    if (width == mWidth && height == mHeight)
        return;

    if (mWidth > UINT32_MAX || mHeight > UINT32_MAX)
    {
        throw std::out_of_range("Invalid width/height");
    }

    if (!mDevice)
        return;

    mWidth = mHeight = 0;

    auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(mFormat,
        static_cast<UINT64>(width),
        static_cast<UINT>(height),
        1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

    D3D12_CLEAR_VALUE clearValue = { mFormat, {} };
    memcpy(clearValue.Color, mClearColor, sizeof(clearValue.Color));

    mState = D3D12_RESOURCE_STATE_RENDER_TARGET;

    // Create a render target
    ThrowIfFailed(
        mDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
            &desc,
            mState, &clearValue,
            IID_PPV_ARGS(mResource.ReleaseAndGetAddressOf()))
    );

    mResource->SetName(L"RenderTexture");

    // Create RTV.
    mDevice->CreateRenderTargetView(mResource.Get(), nullptr, mRtvDescriptor);

    // Create SRV.
    mDevice->CreateShaderResourceView(mResource.Get(), nullptr, mSrvDescriptor);

    mWidth = width;
    mHeight = height;
}

void BaseRenderTexture::ReleaseDevice() noexcept
{
    mResource.Reset();
    mDevice.Reset();

    mState = D3D12_RESOURCE_STATE_COMMON;
    mWidth = mHeight = 0;

    mSrvDescriptor.ptr = mRtvDescriptor.ptr = 0;
}

void BaseRenderTexture::TransitionTo(_In_ ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES afterState)
{
    if (mState == afterState) {
        return;
    }
    auto trans = CD3DX12_RESOURCE_BARRIER::Transition(
        mResource.Get(),
        mState,
        afterState
    );
    commandList->ResourceBarrier(
        1,
        &trans
    );
    mState = afterState;
}

void BaseRenderTexture::SetWindow(const RECT& output)
{
    // Determine the render target size in pixels.
    auto width = size_t(std::max<LONG>(output.right - output.left, 1));
    auto height = size_t(std::max<LONG>(output.bottom - output.top, 1));

    SizeResources(width, height);
}