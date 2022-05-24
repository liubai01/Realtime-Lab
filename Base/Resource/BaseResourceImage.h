#pragma once
#include <windows.h>
#include "../../ThirdParty/d3dx12.h"
#include "DirectXTex.h"
#include <string>
#include <memory>
#include "BaseResource.h"

using namespace std;

class BaseResourceImage : public BaseResource
{
public:
	BaseResourceImage(ID3D12Device* device, const string& path);
	~BaseResourceImage();

	unique_ptr<DirectX::ScratchImage> mScratchImage;

	// handle override methods of BaseResource
	void Upload(ID3D12GraphicsCommandList* commandList);
	void RegisterMainHandle(BaseMainHeap* heap);
	void RegisterRuntimeHandle(BaseRuntimeHeap* heap);
private:
	// D3D12 Wrapper of image buffer in GPU
	ComPtr<ID3D12Resource> mTextureBuffer;
	// The heap used to upload image from CPU to GPU
	ComPtr<ID3D12Resource> mTextureUploadHeap;
	// The resource description of image buffer creation
	D3D12_RESOURCE_DESC GetResourceDesc();
};

