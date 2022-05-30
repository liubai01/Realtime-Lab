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
#include "BaseShaderManager.h"
#include "Asset/BaseAssetManager.h"
#include "../DebugOut.h"
#include "../MathUtils.h"
#include "BaseDirectCommandList.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

using Microsoft::WRL::ComPtr;

class BaseDrawContext: public BaseDirectCommandList
{
public:
	std::vector<D3D12_ROOT_PARAMETER> mRootParams;
	// root params would carry a pointer to descriptor range, which is stored here
	std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> mDescTableRanges;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	std::unique_ptr<BaseShaderManager> mShader;
  
	BaseDrawContext(ID3D12Device* device, BaseAssetManager* assetManager);

	void Init();
	void ResetCommandList();

	ID3D12RootSignature* GetRootSig();

	void AppendCBVDescTable();
	void AppendSRVDescTable();
	
	int mCBVRegCnt;
	int mSRVRegCnt;

	ComPtr<ID3D12RootSignature> mRootSig;
	ComPtr<ID3D12PipelineState> mPSO;
	D3D12_INPUT_LAYOUT_DESC mInputLayoutDesc;

	ID3D12Device* mDevice;
	bool mHasInited;

	void InitRootSig();
	void InitInputLayout();
	virtual void InitPSO();
};

