#include "CoreRenderTextureManager.h"

CoreRenderTextureManager::CoreRenderTextureManager(BaseRuntimeHeap* runtimeHeap, ID3D12Device* device)
{
	mRuntimeHeap = runtimeHeap;
	mDevice = device;
    mSize = 16; // TBD: configurable and could grow
    mCnt = 0;

    // Render target views for the BaseRenderTexture
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = mSize;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    HRESULT hr = device->CreateDescriptorHeap(
        &rtvHeapDesc,
        IID_PPV_ARGS(mRtvDescriptorHeap.GetAddressOf())
    );
    mRtvDescriptorHeap->SetName(L"CoreRenderTextureManager RTV Heap");
}

std::shared_ptr<BaseRenderTexture> CoreRenderTextureManager::AllocateRenderTexture()
{
    std::shared_ptr<BaseRenderTexture> retTexture;
    int rtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.Offset(mCnt++, rtvDescriptorSize);

    XMFLOAT4 clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

    retTexture = std::make_shared<BaseRenderTexture>(DXGI_FORMAT_R8G8B8A8_UNORM);

    retTexture->SetClearColor(XMLoadFloat4(&clearColor));
    retTexture->SetDevice(mDevice, mRuntimeHeap->GetHeapHandleStaticBlock(), rtvHandle);


    return retTexture;
}