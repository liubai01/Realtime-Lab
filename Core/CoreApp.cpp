#include "CoreApp.h"

#include "../Core/Component/CoreMeshComponent.h"
#include "CoreGeometryUtils.h"
#include "../ThirdParty/ImGUI/imgui_internal.h"

CoreApp::CoreApp(HINSTANCE hInstance) : BaseApp(hInstance)
{
  mUploadCmdList = make_unique<BaseDirectCommandList>(mDevice.Get());

  mMaterialManager = make_unique<CoreMaterialManager>(mDevice.Get());
  mLightManager = make_unique<CoreLightManager>(mDevice.Get());

  mLightManager->RegisterMainHandle(mMainHeap);
  mGUIManager = make_unique<CoreGUIManager>(this);

  // Diffuse Draw Context
  mDrawContext = make_unique<BaseDrawContext>(mDevice.Get());
  // Set input layout
  mDrawContext->mInputLayout =
  {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
  };

  mDrawContext->mShader.AddVertexShader("Core\\Shader\\DiffuseVertexShader.hlsl");
  mDrawContext->mShader.AddPixelShader("Core\\Shader\\DiffusePixelShader.hlsl");
  //mMainCamera->SetClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });

  // b0: transform buffer
  // b1: camera buffer
  // b2: matereial buffer
  // b3: light buffer
  for (int i = 0; i < 4; ++i)
  {
      mDrawContext->AppendCBVDescTable();
  }

  // Edge Light Draw Context
  mEdgeLightDrawContext = make_unique<BaseDrawContext>(mDevice.Get());
  // Set input layout
  mEdgeLightDrawContext->mInputLayout =
  {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
  };

  mEdgeLightDrawContext->mShader.AddVertexShader("Core\\Shader\\EdgeLightVertexShader.hlsl");
  mEdgeLightDrawContext->mShader.AddPixelShader("Core\\Shader\\EdgeLightPixelShader.hlsl");

  // b0: transform buffer
  // b1: camera buffer
  for (int i = 0; i < 2; ++i)
  {
      mEdgeLightDrawContext->AppendCBVDescTable();
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

    mMainCamera->BeginScene(commandList);

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

    mMainCamera->EndScene(commandList);
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