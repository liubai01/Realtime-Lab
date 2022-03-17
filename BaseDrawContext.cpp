#include "BaseDrawContext.h"

BaseDrawContext::BaseDrawContext(ID3D12Device* device)
{
  mDevice = device;
  InitCommandList();
}

void BaseDrawContext::InitCommandList()
{
  HRESULT hr = mDevice->CreateCommandAllocator(
    D3D12_COMMAND_LIST_TYPE_DIRECT,
    IID_PPV_ARGS(mCommandAlloc.GetAddressOf())
  );

  ThrowIfFailed(hr);

  hr = mDevice->CreateCommandList(
    0,
    D3D12_COMMAND_LIST_TYPE_DIRECT,
    mCommandAlloc.Get(),
    nullptr,
    IID_PPV_ARGS(mCommandList.GetAddressOf())
  );
  ThrowIfFailed(hr);

  mCommandList->Close();
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
  // Invoke after dumping in Shader, Geometry, etc.
  InitRootSig();
  InitInputLayout();
  InitPSO();
}

void BaseDrawContext::ResetCommandList()
{
  // Reuse the memory associated with command recording.
  // We can only reset when the associated command lists have finished execution on the GPU.
  HRESULT hr = mCommandAlloc->Reset();
  if (FAILED(hr))
  {
    dout::printf("Command Allocated Failed!");
  }

  // A command list can be reset after it has been added to the command queue via ExecuteCommandList.
  // Reusing the command list reuses memory.
  hr = mCommandList->Reset(mCommandAlloc.Get(), mPSO.Get());
  if (FAILED(hr))
  {
    dout::printf("Command List Failed!");
  }
}