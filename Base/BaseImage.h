#pragma once
#include <dxgi.h>
#include <wrl.h>
#include <wincodec.h>
#include <windows.h>
#include "../ThirdParty/d3dx12.h"
#include <dxgi1_4.h>
#include <vector>
#include <string>
#include "../DebugOut.h"
#include "../Base/BaseMainHeap.h"
#include "../Base/BaseRuntimeHeap.h"

using Microsoft::WRL::ComPtr;
using namespace std;


class BaseImage {
public:

	// a name for identification
	string mName;

	// for a lazy upload policy, the variable indicate whether the image has been uploaded before
	bool mIsUploaded;

	BaseImage(ID3D12Device* device, const string& filepath, const string& name);
	~BaseImage();

	void RegisterMainHandle(BaseMainHeap* heap);
	void RegisterRuntimeHandle(BaseRuntimeHeap* heap);

	BaseDescHeapHandle mMainHandle;
	BaseDescHeapHandle mRuntimeHandle;
	
	void Upload(ID3D12GraphicsCommandList* commandList);
private:
	ID3D12Device* mDevice;

	// D3D12 Wrapper of image buffer in GPU
	ComPtr<ID3D12Resource> mTextureBuffer;
	// The heap used to upload image from CPU to GPU
	ComPtr<ID3D12Resource> mTextureUploadHeap;

	// Resource description of image
	D3D12_RESOURCE_DESC mTextureDesc = {};
	int mImageBytesPerRow = -1;
	int mImageSize = -1;
	// Pointer to linear buffer of image in CPU
	BYTE* mImageData = nullptr;
};