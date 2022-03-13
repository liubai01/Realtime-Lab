#pragma once
#include <windows.h>
#include "d3dx12.h"
#include <wrl.h>
#include <vector>
#include <dxgi1_4.h>
#include <unordered_map>
#include <memory>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "ShaderManager.h"
#include "DebugOut.h"
#include "MathUtils.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

using Microsoft::WRL::ComPtr;
using namespace std;

class BaseDrawContext
{
public:
  ComPtr<ID3D12CommandAllocator> mCommandAlloc;
  ComPtr<ID3D12GraphicsCommandList> mCommandList;

  vector<D3D12_ROOT_PARAMETER> mRootParams;
  ComPtr<ID3D12RootSignature> mRootSig;

  vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
  D3D12_INPUT_LAYOUT_DESC mInputLayoutDesc;

  ComPtr<ID3D12PipelineState> mPSO;
  ShaderManager mShader;
  
  BaseDrawContext(ID3D12Device* device);

  void Init();

  void ResetCommandList();

private:
  ID3D12Device* mDevice;

  void InitCommandList();

  void InitRootSig();
  void InitInputLayout();
  void InitPSO();
};

