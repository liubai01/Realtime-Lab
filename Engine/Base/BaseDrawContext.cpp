#include "BaseDrawContext.h"

BaseDrawContext::BaseDrawContext(ID3D12Device* device) : BaseDirectCommandList(device)
{
  mHasInited = false;
  mDevice = device;
  mInputLayoutDesc = {};

  mCBVRegCnt = 0;
  mSRVRegCnt = 0;
}

ID3D12RootSignature* BaseDrawContext::GetRootSig()
{
  Init();

  return mRootSig.Get();
}

void BaseDrawContext::InitRootSig()
{
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc;

    // create a static sampler
    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.MipLODBias = 0;
    sampler.MaxAnisotropy = 0;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD = 0.0f;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = 0;
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    rootSigDesc.Init(
    mRootParams.size(),
    mRootParams.data(),
    1,
    &sampler,
    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );

    ComPtr<ID3DBlob> signature;
    HRESULT hr = D3D12SerializeRootSignature(
    &rootSigDesc,
    D3D_ROOT_SIGNATURE_VERSION_1,
    signature.GetAddressOf(),
    nullptr
    );
    if (FAILED(hr))
    {
        dout::printf("Signature Blob(CPU) Failed");
    }

    hr = mDevice->CreateRootSignature(
    0,
    signature->GetBufferPointer(),
    signature->GetBufferSize(),
    IID_PPV_ARGS(mRootSig.GetAddressOf())
    );
    if (FAILED(hr))
    {
        dout::printf("RootSignature Created Failed!");
    }

}

void BaseDrawContext::InitInputLayout()
{
  // fill out an input layout description structure
  mInputLayoutDesc = {};

  // we can get the number of elements in an array by "sizeof(array) / sizeof(arrayElementType)"
  mInputLayoutDesc.NumElements = mInputLayout.size();
  mInputLayoutDesc.pInputElementDescs = mInputLayout.data();
}

void BaseDrawContext::InitPSO()
{
  // create a pipeline state object (PSO)

  // In a real application, you will have many pso's. for each different shader
  // or different combinations of shaders, different blend states or different rasterizer states,
  // different topology types (point, line, triangle, patch), or a different number
  // of render targets you will need a pso

  // VS is the only required shader for a pso. You might be wondering when a case would be where
  // you only set the VS. It's possible that you have a pso that only outputs data with the stream
  // output, and not on a render target, which means you would not need anything after the stream
  // output.

  DXGI_SAMPLE_DESC sampleDesc = {};
  sampleDesc.Count = 1;

  D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};   // a structure to define a pso
  psoDesc.InputLayout = mInputLayoutDesc;            // the structure describing our input layout
  psoDesc.pRootSignature = mRootSig.Get();           // the root signature that describes the input data this pso needs
  psoDesc.VS = mShader.VertexShaderByteCode();               // structure describing where to find the vertex shader bytecode and how large it is
  psoDesc.PS = mShader.PixelShaderByteCode();                // same as VS but for pixel shader
  psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
  psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;        // format of the render target
  psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
  psoDesc.SampleDesc = sampleDesc;                           // must be the same sample description as the swapchain and depth/stencil buffer
  psoDesc.SampleMask = 0xffffffff;                           // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
  psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // a default rasterizer state.
  psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
  psoDesc.NumRenderTargets = 1; // we are only binding one render target
  psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // a default depth stencil state

  // create the PSO
  HRESULT hr = mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(mPSO.GetAddressOf()));
  ThrowIfFailed(hr);

}

void BaseDrawContext::Init()
{
  if (!mHasInited)
  {
    mHasInited = true;
    // Invoke after dumping in Shader, Geometry, etc.
    InitRootSig();
    InitInputLayout();
    InitPSO();
  }
}

void BaseDrawContext::ResetCommandList()
{
  Init();
  // Overload because PSO should be carried when resetting commandlist 
  HRESULT hr = mCommandAlloc->Reset();
  if (FAILED(hr))
  {
    dout::printf("[BaseDrawContext] Command Allocated Failed!");
  }
  ThrowIfFailed(hr);

  hr = mCommandList->Reset(mCommandAlloc.Get(), mPSO.Get());
  if (FAILED(hr))
  {
    dout::printf("[BaseDrawContext] Command List Failed!");
  }
  ThrowIfFailed(hr);
}

void BaseDrawContext::AppendCBVDescTable()
{
    mDescTableRanges.emplace_back(1);
    vector<D3D12_DESCRIPTOR_RANGE>& descTableRanges = mDescTableRanges.back();

    descTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; // this is a range of constant buffer views (descriptors)
    descTableRanges[0].NumDescriptors = 1; // we only have one constant buffer, so the range is only 1
    descTableRanges[0].BaseShaderRegister = mCBVRegCnt++; // start index of the shader registers in the range
    descTableRanges[0].RegisterSpace = 0; // space 0. can usually be zero
    descTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // this appends the range to the end of the root signature descriptor tables

    // create a descriptor table
    D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
    descriptorTable.NumDescriptorRanges = static_cast<UINT>(descTableRanges.size()); // we only have one range
    descriptorTable.pDescriptorRanges = descTableRanges.data(); // the pointer to the beginning of our ranges array

    // create a root parameter and fill it out
    mRootParams.emplace_back();
    mRootParams.back().ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // this is a descriptor table
    mRootParams.back().DescriptorTable = descriptorTable; // this is our descriptor table for this root parameter
    mRootParams.back().ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
}

void BaseDrawContext::AppendSRVDescTable()
{
    mDescTableRanges.emplace_back(1);
    vector<D3D12_DESCRIPTOR_RANGE>& descTableRanges = mDescTableRanges.back();

    descTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // this is a range of constant buffer views (descriptors)
    descTableRanges[0].NumDescriptors = 1; // we only have one constant buffer, so the range is only 1
    descTableRanges[0].BaseShaderRegister = mSRVRegCnt++; // start index of the shader registers in the range
    descTableRanges[0].RegisterSpace = 0; // space 0. can usually be zero
    descTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // this appends the range to the end of the root signature descriptor tables

    // create a descriptor table
    D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
    descriptorTable.NumDescriptorRanges = static_cast<UINT>(descTableRanges.size()); // we only have one range
    descriptorTable.pDescriptorRanges = descTableRanges.data(); // the pointer to the beginning of our ranges array

    // create a root parameter and fill it out
    mRootParams.emplace_back();
    mRootParams.back().ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // this is a descriptor table
    mRootParams.back().DescriptorTable = descriptorTable; // this is our descriptor table for this root parameter
    mRootParams.back().ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
}