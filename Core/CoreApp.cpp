#include "CoreApp.h"

#include "../Core/Component/CoreMeshComponent.h"
#include "CoreGeometryUtils.h"
#include "../ThirdParty/ImGUI/imgui_internal.h"

CoreApp::CoreApp(HINSTANCE hInstance) : BaseApp(hInstance)
{
  mDrawContext = make_unique<BaseDrawContext>(mDevice.Get());
  mUploadCmdList = make_unique<BaseDirectCommandList>(mDevice.Get());

  mMaterialManager = make_unique<CoreMaterialManager>(mDevice.Get());
  mLightManager = make_unique<CoreLightManager>(mDevice.Get());

  mLightManager->RegisterMainHandle(mMainHeap);
  mGUIManager = make_unique<CoreGUIManager>(this);

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
  mGUIManager->Update();

  // Set resource to runtime heap
  mRuntimeHeap->Reset();

  // Register Runtime Heap
  mMaterialManager->RegisterRuntimeHandle(mRuntimeHeap);
  mLightManager->RegisterRuntimeHandle(mRuntimeHeap);
  mGOManager->DispatchTransformUpload(mRuntimeHeap);
  mMainCamera->RegisterRuntimeHandle(mRuntimeHeap);

  UploadGeometry();

  mDrawContext->ResetCommandList();
  ID3D12GraphicsCommandList* commandList = mDrawContext->mCommandList.Get();
  auto trans = CD3DX12_RESOURCE_BARRIER::Transition(
      mRenderTargets[mFrameIdx].Get(),
      D3D12_RESOURCE_STATE_PRESENT,
      D3D12_RESOURCE_STATE_RENDER_TARGET
  );
  commandList->ResourceBarrier(
      1,
      &trans
  );

  RenderObjects();
  mGUIManager->Render(mDrawContext->mCommandList.Get());

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
  Swap();
}

void CoreApp::UploadGeometry()
{
    // Upload geoemetry(vertices, indices) if it has not been uploaded before
    mUploadCmdList->ResetCommandList();

    for (auto& elem : mGOManager->mObjs)
    {
        shared_ptr<BaseObject>& obj = elem.second;

        for (auto component : obj->mComponents)
        {
            if (component->mComponentType == BaseComponentType::BASE_COMPONENT_MESH)
            {
                CoreMeshComponent* com = static_cast<CoreMeshComponent*>(&(*component));
                // component could be shared by multiple objects
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
    ID3D12GraphicsCommandList* commandList = mDrawContext->mCommandList.Get();

    mMainCamera->BeginScene(commandList, mFrameIdx);

    commandList->SetGraphicsRootSignature(mDrawContext->GetRootSig());

    ID3D12DescriptorHeap* descriptorHeaps[] = { mRuntimeHeap->mDescHeap.Get() };
    commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    
    commandList->SetGraphicsRootDescriptorTable(1, mMainCamera->GetHandle().GetGPUHandle());
    commandList->SetGraphicsRootDescriptorTable(3, mLightManager->mLightData.GetHandle().GetGPUHandle());

    for (auto& elem : mGOManager->mObjs)
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

    mMainCamera->EndScene(commandList, mFrameIdx);
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