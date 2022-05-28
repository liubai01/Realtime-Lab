#pragma once
#include <windows.h>
#include "../../ThirdParty/d3dx12.h"
#include "DirectXTex.h"
#include <string>
#include <memory>
#include "BaseResource.h"
#include "../../ThirdParty/ImGUI/imgui.h"

using namespace std;

class BaseResourceImage : public BaseResource
{
public:
	static const BaseResourceType ClassResourceType = BaseResourceType::RESOURCE_IMAGE;
	static const BaseAssetType ClassAssetType = BaseAssetType::ASSET_IMAGE;

	BaseResourceImage(ID3D12Device* device, BaseAssetNode* assetNode);
	~BaseResourceImage();

	unique_ptr<DirectX::ScratchImage> mScratchImage;

	// For Dear ImGui in Editor
	ImTextureID GetImGuiRuntimeID();

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

