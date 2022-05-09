#pragma once
#include "../Base/BaseRuntimeHeap.h"
#include "../Base/BaseUUIDGenerator.h"
#include "../Base/BaseRenderTexture.h"
#include <memory>

class CoreRenderTextureManager
{
public:
	CoreRenderTextureManager(BaseRuntimeHeap* UIRuntimeHeap, ID3D12Device* device);
	std::shared_ptr<BaseRenderTexture> AllocateRenderTexture();
private:
	BaseRuntimeHeap* mUIRuntimeHeap;
	ComPtr<ID3D12DescriptorHeap> mRtvDescriptorHeap;
	ID3D12Device* mDevice;

	int mSize; // the size of descriptor heap
	int mCnt; // number of render texture in heap
};

