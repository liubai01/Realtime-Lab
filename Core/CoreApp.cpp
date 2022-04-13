#include "CoreApp.h"

#include "../Core/Component/CoreMeshComponent.h"
#include "CoreGeometryUtils.h"

CoreApp::CoreApp(HINSTANCE hInstance) : BaseApp(hInstance)
{
  mDrawContext = make_unique<BaseDrawContext>(mDevice.Get());
  mUploadCmdList = make_unique<BaseDirectCommandList>(mDevice.Get());
  mMaterialManager = make_unique<CoreMaterialManager>(mDevice.Get());
  mLightManager = make_unique<CoreLightManager>(mDevice.Get());

  mLightManager->RegisterMainHandle(mMainHeap);

  // Set input layout
  mDrawContext->mInputLayout =
  {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
  };

  mDrawContext->mShader.AddVertexShader("Core\\Shader\\ShadowMapVertexShader.hlsl");
  mDrawContext->mShader.AddPixelShader("Core\\Shader\\ShadowMapPixelShader.hlsl");

  // b0: transform buffer
  // b1: camera buffer
  // b2: matereial buffer
  // b3: light buffer
  for (int i = 0; i < 4; ++i)
  {
    mDrawContext->mDescTableRanges.emplace_back(1);
    vector<D3D12_DESCRIPTOR_RANGE>& descTableRanges = mDrawContext->mDescTableRanges.back();

    descTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; // this is a range of constant buffer views (descriptors)
    descTableRanges[0].NumDescriptors = 1; // we only have one constant buffer, so the range is only 1
    descTableRanges[0].BaseShaderRegister = i; // start index of the shader registers in the range
    descTableRanges[0].RegisterSpace = 0; // space 0. can usually be zero
    descTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // this appends the range to the end of the root signature descriptor tables

    // create a descriptor table
    D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
    descriptorTable.NumDescriptorRanges = static_cast<UINT>(descTableRanges.size()); // we only have one range
    descriptorTable.pDescriptorRanges = descTableRanges.data(); // the pointer to the beginning of our ranges array

    // create a root parameter and fill it out
    mDrawContext->mRootParams.emplace_back();
    mDrawContext->mRootParams.back().ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // this is a descriptor table
    mDrawContext->mRootParams.back().DescriptorTable = descriptorTable; // this is our descriptor table for this root parameter
    mDrawContext->mRootParams.back().ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
  }
}

void CoreApp::Start() 
{

}

void CoreApp::Update()
{

}


void CoreApp::Render()
{
  // Set resource to runtime heap
  mRuntimeHeap->Reset();
  mMaterialManager->RegisterRuntimeHandle(mRuntimeHeap);
  mLightManager->RegisterRuntimeHandle(mRuntimeHeap);

  UploadGeometry();
  RenderObjects();
  Swap();
}

void CoreApp::UploadGeometry()
{
    // 1. Upload geoemetry(vertices, indices) if it has not been uploaded before
    // 2. Register transform constant buffer
    mUploadCmdList->ResetCommandList();
    for (auto& elem : *mObjs)
    {
        shared_ptr<BaseObject>& obj = elem.second;
        obj->mTransform.RegisterRuntimeHandle(mRuntimeHeap);

        for (auto component : obj->mComponents)
        {
            if (component->mComponentType == BaseComponentType::BASE_COMPONENT_MESH)
            {
                CoreMeshComponent* com = static_cast<CoreMeshComponent*>(&(*component));
                if (!com->mUploaded)
                {
                    com->Upload(mDevice.Get(), mUploadCmdList->mCommandList.Get());
                }
            }
        }
    }
    Enqueue(mUploadCmdList->mCommandList.Get());
}

void CoreApp::RenderObjects()
{
    mDrawContext->ResetCommandList();
    ID3D12GraphicsCommandList* commandList = mDrawContext->mCommandList.Get();

    commandList->SetGraphicsRootSignature(mDrawContext->GetRootSig());
    commandList->RSSetViewports(1, &mMainCamera->mViewport);
    commandList->RSSetScissorRects(1, &mMainCamera->mScissorRect);

    auto trans = CD3DX12_RESOURCE_BARRIER::Transition(
        mRenderTargets[mFrameIdx].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    commandList->ResourceBarrier(
        1,
        &trans
    );

    // Clear the back buffer and depth buffer.
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DepthBufferView();

    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    commandList->OMSetRenderTargets(1, &rtv, false, &dsv);

    ID3D12DescriptorHeap* descriptorHeaps[] = { mRuntimeHeap->mDescHeap.Get() };
    commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    mMainCamera->RegisterRuntimeHandle(mRuntimeHeap);
    commandList->SetGraphicsRootDescriptorTable(1, mMainCamera->GetHandle().GetGPUHandle());
    commandList->SetGraphicsRootDescriptorTable(3, mLightManager->mLightData.GetHandle().GetGPUHandle());

    for (auto& elem : *mObjs)
    {
        shared_ptr<BaseObject>& obj = elem.second;
        commandList->SetGraphicsRootDescriptorTable(0, obj->mTransform.GetHandle().GetGPUHandle());
        for (auto component : obj->mComponents)
        {
            if (component->mComponentType == BaseComponentType::BASE_COMPONENT_MESH)
            {
                CoreMeshComponent* com = static_cast<CoreMeshComponent*>(&(*component));
                commandList->SetGraphicsRootDescriptorTable(2, com->mMat->GetHandle().GetGPUHandle());

                D3D12_INDEX_BUFFER_VIEW ibView = com->mIndexBufferView;
                D3D12_VERTEX_BUFFER_VIEW vbView = com->mVertexBufferView;

                commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                commandList->IASetVertexBuffers(0, 1, &vbView);
                commandList->IASetIndexBuffer(&ibView);

                commandList->DrawIndexedInstanced(
                    static_cast<UINT>(com->mGeo->mIndices.size()),
                    1, 0, 0, 0
                );
            }
        }
    }


    trans = CD3DX12_RESOURCE_BARRIER::Transition(
        mRenderTargets[mFrameIdx].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT
    );
    commandList->ResourceBarrier(
        1,
        &trans
    );
    Flush(mDrawContext->mCommandList.Get());
}

shared_ptr<CoreMaterial> CoreApp::CreateMaterial(const string& name)
{
    shared_ptr<CoreMaterial> mat = mMaterialManager->CreateMaterial(name);
    if (mat != nullptr)
    {
        mat->RegisterMainHandle(mMainHeap);
    }

    return mat;
}