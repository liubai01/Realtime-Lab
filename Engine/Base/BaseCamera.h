#pragma once
#include <windows.h>
#include "../ThirdParty/d3dx12.h"
#include <wrl.h>
#include <vector>
#include <dxgi1_4.h>
#include <unordered_map>
#include <memory>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "../DebugOut.h"
#include "../MathUtils.h"
#include <time.h>
#include "BaseStagedBuffer.h"
#include "BaseRenderTexture.h"
#include "BaseRuntimeHeap.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

using Microsoft::WRL::ComPtr;

struct BaseCameraConstant {
  XMFLOAT4X4 ViewProj = Identity4x4();
  XMFLOAT4 EyePos = { 0.0f, 0.0f, 0.0f, 1.0f };
  XMFLOAT4 WindowSize = { 0.0f, 0.0f, 0.0f, 0.0f };
};

class BaseCamera: public BaseStagedBuffer<BaseCameraConstant>
{
public:
	BaseCamera(ID3D12Device* device, float width, float height, float nearZ=1.0f, float FarZ=1000.0f);

	void SetPos(float x, float y, float z);
	void SetSize(float width, float height);

	void SetRenderTexture(std::shared_ptr<BaseRenderTexture> renderTexture);
	void SetDepthWrite(bool depthOn);

	void BeginScene(ID3D12GraphicsCommandList* commandList);
	void EndScene(ID3D12GraphicsCommandList* commandList);

	const BaseDescHeapHandle& GetRenderTextureSRVHandle();

	float mWidth;
	float mHeight;

	void Upload();

	D3D12_VIEWPORT mViewport;    // area that output from rasterizer will be stretched to.
	D3D12_RECT mScissorRect;     // the area to draw in. pixels outside that area will not be drawn onto

private:
	ID3D12Device* mDevice;

	XMFLOAT4X4 mProj;
	XMFLOAT4 mPos;

	float mNearZ;
	float mFarZ;

	// depth buffer
	ComPtr<ID3D12Resource> mDepthStencilBuffer;
	ComPtr<ID3D12DescriptorHeap> mDepthDescriptorHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthBufferView() const;
	bool mDepthOn;

	// render texture
	std::shared_ptr<BaseRenderTexture> mRenderTexture;
};

