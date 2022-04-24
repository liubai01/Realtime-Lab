#include "BaseCamera.h"

BaseCamera::BaseCamera(ID3D12Device* device, BaseRuntimeHeap* mUIHeap, float width, float height, int frameCnt, float nearZ, float farZ) : BaseStagedBuffer(device)
{
    mFrameCnt = frameCnt;
    mNearZ = nearZ;
    mFarZ = farZ;

    XMVECTOR pos = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    XMStoreFloat4(&mPos, pos);

    // Render target views for the BaseRenderTexture
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = frameCnt;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    HRESULT hr = device->CreateDescriptorHeap(
        &rtvHeapDesc,
        IID_PPV_ARGS(mRtvDescriptorHeap.GetAddressOf())
    );

    int rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0; i < frameCnt; ++i)
    {
        unique_ptr<BaseRenderTexture> rt = make_unique<BaseRenderTexture>(DXGI_FORMAT_R8G8B8A8_UNORM);
        BaseDescHeapHandle runtimeHandle = mUIHeap->GetHeapHandleBlock(1);
        mRTHandles.push_back(runtimeHandle);

        rt->SetClearColor({ 0.0f, 0.2f, 0.4f, 1.0f });
        rt->SetDevice(device, runtimeHandle.GetCPUHandle(), rtvHandle);

        mRenderTextures.emplace_back(move(rt));
        rtvHandle.Offset(1, rtvDescriptorSize);
    }


    SetSize(width, height);
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

    for (int i = 0; i < mFrameCnt; ++i)
    {
        BaseRenderTexture* rt = &*mRenderTextures[i];
        rt->SetWindow(mScissorRect);
    }
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
}