#include "BaseResourceImage.h"
#include "../../DebugOut.h"

BaseResourceImage::BaseResourceImage(ID3D12Device* device, BaseAssetNode* assetNode) : BaseResource(device, assetNode)
{
    // for runtime deciding the acutal type of BaseResource base pointer
    mType = BaseResourceImage::ClassResourceType;
    mIsRuntimeResource = true;

    // for invoking LoadFromWICFile, convert path of image to wide char string 
    std::wstring widepath = std::wstring(assetNode->mFullPath.begin(), assetNode->mFullPath.end());

    mScratchImage = std::make_unique<DirectX::ScratchImage>();
    HRESULT hr = LoadFromWICFile(widepath.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, *mScratchImage);
    ThrowIfFailed(hr);

    D3D12_RESOURCE_DESC resourceDesc = GetResourceDesc();
    // create a default heap where the upload heap will copy its contents into (contents being the texture)
    auto hprop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    hr = mDevice->CreateCommittedResource(
        &hprop, // a default heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &resourceDesc, // the description of our texture
        D3D12_RESOURCE_STATE_COPY_DEST, // We will copy the texture from the upload heap to here, so we start it out in a copy dest state
        nullptr, // used for render targets and depth/stencil buffers
        IID_PPV_ARGS(mTextureBuffer.GetAddressOf())
    );
    ThrowIfFailed(hr);
    mTextureBuffer->SetName(L"Texture Buffer Resource Heap");

    UINT64 textureUploadBufferSize;
    // this function gets the size an upload buffer needs to be to upload a texture to the gpu.
    // each row must be 256 byte aligned except for the last row, which can just be the size in bytes of the row
    // eg. textureUploadBufferSize = ((((width * numBytesPerPixel) + 255) & ~255) * (height - 1)) + (width * numBytesPerPixel);
    //textureUploadBufferSize = (((imageBytesPerRow + 255) & ~255) * (textureDesc.Height - 1)) + imageBytesPerRow;
    mDevice->GetCopyableFootprints(&resourceDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

    // now we create an upload heap to upload our texture to the GPU
    hprop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto rdesc = CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize);
    hr = mDevice->CreateCommittedResource(
        &hprop, // upload heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &rdesc, // resource description for a buffer (storing the image data in this heap just to copy to the default heap)
        D3D12_RESOURCE_STATE_GENERIC_READ, // We will copy the contents from this heap to the default heap above
        nullptr,
        IID_PPV_ARGS(mTextureUploadHeap.GetAddressOf()));
    ThrowIfFailed(hr);
    mTextureUploadHeap->SetName(L"Texture Buffer Upload Resource Heap");
}

BaseResourceImage::~BaseResourceImage()
{
}

D3D12_RESOURCE_DESC BaseResourceImage::GetResourceDesc()
{
    DirectX::TexMetadata imageMeta = mScratchImage->GetMetadata();

    D3D12_RESOURCE_DESC resourceDescription = {};
    resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDescription.Alignment = 0; // may be 0, 4KB, 64KB, or 4MB. 0 will let runtime decide between 64KB and 4MB (4MB for multi-sampled textures)
    resourceDescription.Width = static_cast<UINT>(imageMeta.width); // width of the texture
    resourceDescription.Height = static_cast<UINT>(imageMeta.height); // height of the texture
    resourceDescription.DepthOrArraySize = 1; // if 3d image, depth of 3d image. Otherwise an array of 1D or 2D textures (we only have one image, so we set 1)
    resourceDescription.MipLevels = 1; // Number of mipmaps. We are not generating mipmaps for this texture, so we have only one level
    resourceDescription.Format = imageMeta.format; // This is the dxgi format of the image (format of the pixels)
    resourceDescription.SampleDesc.Count = 1; // This is the number of samples per pixel, we just want 1 sample
    resourceDescription.SampleDesc.Quality = 0; // The quality level of the samples. Higher is better quality, but worse performance
    resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // The arrangement of the pixels. Setting to unknown lets the driver choose the most efficient one
    resourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE; // no flags

    return resourceDescription;
}

void BaseResourceImage::Upload(ID3D12GraphicsCommandList* commandList)
{
    DirectX::TexMetadata imageMeta = mScratchImage->GetMetadata();
    int bytesPerRow = static_cast<int>(DirectX::BitsPerPixel(imageMeta.format)) * static_cast<int>(imageMeta.width) / 8;

    // store vertex buffer in upload heap
    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = mScratchImage->GetPixels(); // pointer to our image data
    textureData.RowPitch = bytesPerRow;
    textureData.SlicePitch = bytesPerRow * imageMeta.height;

    // Now we copy the upload buffer contents to the default heap
    UpdateSubresources(commandList, mTextureBuffer.Get(), mTextureUploadHeap.Get(), 0, 0, 1, &textureData);

    // transition the texture default heap to a pixel shader resource (we will be sampling from this heap in the pixel shader to get the color of pixels)
    auto trans = CD3DX12_RESOURCE_BARRIER::Transition(
        mTextureBuffer.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    commandList->ResourceBarrier(1, &trans);

    // the data in the CPU would be no longer used
    mScratchImage.reset();

    mUpload = true;
}

void BaseResourceImage::RegisterMainHandle(BaseMainHeap* mainHeap)
{
    DirectX::TexMetadata imageMeta = mScratchImage->GetMetadata();
    mMainHandle = BaseDescHeapHandle(mainHeap->GetNewHeapHandle());

    // now we create a shader resource view (descriptor that points to the texture and describes it)
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = imageMeta.format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    mDevice->CreateShaderResourceView(mTextureBuffer.Get(), &srvDesc, mMainHandle.GetCPUHandle());
}

void BaseResourceImage::RegisterRuntimeHandle(BaseRuntimeHeap* heap)
{
    mRuntimeHandle = heap->GetHeapHandleBlock(1);
    mDevice->CopyDescriptorsSimple(
        1,
        mRuntimeHandle.GetCPUHandle(),
        mMainHandle.GetCPUHandle(),
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
    );
}


ImTextureID BaseResourceImage::GetImGuiRuntimeID()
{
    return (ImTextureID) (mRuntimeHandle.GetGPUHandle().ptr);
}