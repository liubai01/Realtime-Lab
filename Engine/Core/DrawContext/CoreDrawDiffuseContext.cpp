#include "CoreDrawDiffuseContext.h"
#include "../CoreGeometry.h"

CoreDrawDiffuseContext::CoreDrawDiffuseContext(ID3D12Device* device) : BaseDrawContext(device)
{
    mInputLayout = CoreGeometry::GetInputLayout();

    mShader.AddVertexShader("Engine\\Core\\Shader\\DiffuseVertexShader.hlsl");
    mShader.AddPixelShader("Engine\\Core\\Shader\\DiffusePixelShader.hlsl");

    // 0, b0: transform buffer
    // 1, b1: camera buffer
    // 2, b2: matereial buffer
    // 3, b3: light buffer
    // 4, t0: diffuse color texture
    // 5, t1: normal map texture
    for (int i = 0; i < 4; ++i)
    {
        AppendCBVDescTable();
    }
    AppendSRVDescTable(); // base color texture
    AppendSRVDescTable(); // normal texture
}

void CoreDrawDiffuseContext::InitPSO()
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