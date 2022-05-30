#include "BaseCamera.h"

BaseCamera::BaseCamera(ID3D12Device* device, float width, float height, float nearZ, float farZ) : BaseStagedBuffer(device)
{
    mNearZ = nearZ;
    mFarZ = farZ;
    mDevice = device;

    // Depth desc heap
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    HRESULT hr = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(mDepthDescriptorHeap.GetAddressOf()));
    if (FAILED(hr))
    {
        dout::printf("[BaseCamera] Fail to create depth texture");
    }

    mDepthDescriptorHeap->SetName(L"Depth/Stencil Resource Heap");
    mDepthOn = true;

    SetSize(width, height);
}

void BaseCamera::SetDepthWrite(bool depthOn)
{
    mDepthOn = depthOn;
}

void BaseCamera::SetRenderTexture(std::shared_ptr<BaseRenderTexture> renderTexture)
{
    mRenderTexture = renderTexture;
    if (mRenderTexture->mHeight != mHeight || mRenderTexture->mWidth != mWidth)
    {
        mRenderTexture->SetWindow(mScissorRect);
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE BaseCamera::DepthBufferView() const
{
  CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(mDepthDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
  return dsvHandle;
}

void BaseCamera::SetSize(float width, float height)
{
    mWidth = width;
    mHeight = height;

    // Fill out the Viewport
    mViewport.TopLeftX = 0;
    mViewport.TopLeftY = 0;
    mViewport.Width = static_cast<float>(width);
    mViewport.Height = static_cast<float>(height);
    mViewport.MinDepth = 0.0f;
    mViewport.MaxDepth = 1.0f;

    // Fill out a scissor rect
    mScissorRect.left = 0;
    mScissorRect.top = 0;
    mScissorRect.right = static_cast<long>(width);
    mScissorRect.bottom = static_cast<long>(height);

    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * 3.1415926535f, static_cast<float>(width) / height, mNearZ, mFarZ);
    XMStoreFloat4x4(&mProj, P);

    if (mRenderTexture != nullptr)
    {
        mRenderTexture->SetWindow(mScissorRect);
    }

    auto hprop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto rdesc = CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_D32_FLOAT,
        static_cast<UINT64>(width),
        static_cast<UINT>(height), 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
    );

    D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
    depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;

    mDevice->CreateCommittedResource(
        &hprop,
        D3D12_HEAP_FLAG_NONE,
        &rdesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthOptimizedClearValue,
        IID_PPV_ARGS(mDepthStencilBuffer.ReleaseAndGetAddressOf())
    );

    D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
    depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

    mDevice->CreateDepthStencilView(
        mDepthStencilBuffer.Get(),
        &depthStencilDesc,
        mDepthDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
    );

    // set an initialize value away from zero point
    // the default look at (0, 0, 0) would raise error if the camera is at the same point
    SetPos(10.0f, 10.0f, -10.0f);
}

void BaseCamera::SetPos(float x, float y, float z)
{
    mPos.x = x;
    mPos.y = y;
    mPos.z = z;
}

void BaseCamera::Upload()
{
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMVECTOR pos = XMLoadFloat4(&mPos);
    
    XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);

    XMMATRIX viewproj = view * proj;

    XMStoreFloat4x4(&mBuffer.mData.ViewProj, XMMatrixTranspose(viewproj));
    mBuffer.mData.EyePos = mPos;

    mBuffer.mData.WindowSize.y = mHeight;
    mBuffer.mData.WindowSize.x = mWidth;
}

void BaseCamera::BeginScene(ID3D12GraphicsCommandList* commandList)
{
    mRenderTexture->BeginScene(commandList);

    commandList->RSSetViewports(1, &mViewport);
    commandList->RSSetScissorRects(1, &mScissorRect);

    if (mDepthOn)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtv = mRenderTexture->mRtvDescriptor;
        D3D12_CPU_DESCRIPTOR_HANDLE dsv = DepthBufferView();

        commandList->ClearRenderTargetView(rtv, mRenderTexture->mClearColor, 0, nullptr);
        commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

        commandList->OMSetRenderTargets(1, &rtv, false, &dsv);
    }
    else {
        D3D12_CPU_DESCRIPTOR_HANDLE rtv = mRenderTexture->mRtvDescriptor;

        commandList->ClearRenderTargetView(rtv, mRenderTexture->mClearColor, 0, nullptr);

        commandList->OMSetRenderTargets(1, &rtv, false, nullptr);
    }

}

void BaseCamera::EndScene(ID3D12GraphicsCommandList* commandList)
{
    mRenderTexture->EndScene(commandList);
}

const BaseDescHeapHandle& BaseCamera::GetRenderTextureSRVHandle()
{
    return mRenderTexture->mSRVHandle;
}