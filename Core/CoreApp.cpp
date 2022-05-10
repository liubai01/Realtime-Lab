#include "CoreApp.h"

#include "../Core/Component/CoreMeshComponent.h"
#include "CoreGeometryUtils.h"
#include "../ThirdParty/ImGUI/imgui_internal.h"
#include "../ThirdParty/ImGUI/imgui.h"

CoreApp::CoreApp(HINSTANCE hInstance) : BaseApp(hInstance)
{
  mUploadCmdList = make_unique<BaseDirectCommandList>(mDevice.Get());
  mUIDrawCmdList = make_unique<BaseDirectCommandList>(mDevice.Get());

  mMaterialManager = make_unique<CoreMaterialManager>(mDevice.Get());
  mLightManager = make_unique<CoreLightManager>(mDevice.Get());

  mLightManager->RegisterMainHandle(mMainHeap);
  mGUIManager = make_unique<CoreGUIManager>(this);

  mRenderTextureManager = make_unique<CoreRenderTextureManager>(mRuntimeHeap, mDevice.Get());
  mSceneRenderTexture = mRenderTextureManager->AllocateRenderTexture();
  mEdgeRenderTexture = mRenderTextureManager->AllocateRenderTexture();

  shared_ptr<CoreGeometry> planeGeo = make_shared<CoreGeometry>(GetPlaneGeometry());
  mFullScreenPlane = make_unique<CoreMeshComponent>(planeGeo);

  mMainCamera->SetRenderTexture(mSceneRenderTexture);

  // Diffuse Draw Context
  mDrawContext = make_unique<CoreDrawDiffuseContext>(mDevice.Get());

  // Edge Light Draw Context
  mEdgeLightDrawContext = make_unique<CoreDrawEdgeContext>(mDevice.Get());
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

  // Register Runtime Heap
  mMaterialManager->RegisterRuntimeHandle(mRuntimeHeap);
  mLightManager->RegisterRuntimeHandle(mRuntimeHeap);
  mGOManager->DispatchTransformUpload(mRuntimeHeap);
  mMainCamera->RegisterRuntimeHandle(mRuntimeHeap);

  UploadGeometry();
  RenderEdgeLightPre();
  RenderObjects();
  RenderUI();
  
  Swap();
}

void CoreApp::BeforeUpdate()
{
    mGUIManager->Update();
    //ImGui::ShowDemoWindow();
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

    // this full screen plane would be used when lighting edge
    if (!mFullScreenPlane->mUploaded)
    {
        mFullScreenPlane->Upload(mDevice.Get(), mUploadCmdList->mCommandList.Get());
    }
    
    Enqueue(mUploadCmdList->mCommandList.Get());
}

void CoreApp::RenderEdgeLightPre()
{
    mEdgeLightDrawContext->ResetCommandList();
    ID3D12GraphicsCommandList* commandList = mEdgeLightDrawContext->mCommandList.Get();

    mMainCamera->SetRenderTexture(mEdgeRenderTexture);
    mMainCamera->SetDepthWrite(false);
    mMainCamera->BeginScene(commandList);

    commandList->SetGraphicsRootSignature(mEdgeLightDrawContext->GetRootSig());

    ID3D12DescriptorHeap* descriptorHeaps[] = { mRuntimeHeap->mDescHeap.Get() };
    commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    commandList->SetGraphicsRootDescriptorTable(1, mMainCamera->GetRuntimeHandle().GetGPUHandle());

    for (auto& elem : mGOManager->mObjs)
    {
        shared_ptr<BaseObject>& obj = elem.second;
        commandList->SetGraphicsRootDescriptorTable(0, obj->mTransform.GetRuntimeHandle().GetGPUHandle());
        for (auto component : obj->mComponents)
        {
            if (component->mComponentType == BaseComponentType::BASE_COMPONENT_MESH)
            {
                CoreMeshComponent* com = static_cast<CoreMeshComponent*>(&(*component));

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
    Enqueue(mEdgeLightDrawContext->mCommandList.Get());
}

void CoreApp::RenderUI()
{
    mUIDrawCmdList->ResetCommandList();
    ID3D12GraphicsCommandList* commandList = mUIDrawCmdList->mCommandList.Get();
    auto trans = CD3DX12_RESOURCE_BARRIER::Transition(
        mRenderTargets[mFrameIdx].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    commandList->ResourceBarrier(
        1,
        &trans
    );

    mGUIManager->Render(commandList);

    trans = CD3DX12_RESOURCE_BARRIER::Transition(
        mRenderTargets[mFrameIdx].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT
    );
    commandList->ResourceBarrier(
        1,
        &trans
    );

    Flush(commandList); // this is the last step
}

void CoreApp::RenderObjects()
{
    mDrawContext->ResetCommandList();
    ID3D12GraphicsCommandList* commandList = mDrawContext->mCommandList.Get();

    mMainCamera->SetRenderTexture(mSceneRenderTexture);
    mMainCamera->SetDepthWrite(true);
    mMainCamera->BeginScene(commandList);

    commandList->SetGraphicsRootSignature(mDrawContext->GetRootSig());

    ID3D12DescriptorHeap* descriptorHeaps[] = { mRuntimeHeap->mDescHeap.Get() };
    commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    
    commandList->SetGraphicsRootDescriptorTable(1, mMainCamera->GetRuntimeHandle().GetGPUHandle());
    commandList->SetGraphicsRootDescriptorTable(3, mLightManager->mLightData.GetRuntimeHandle().GetGPUHandle());

    for (auto& elem : mGOManager->mObjs)
    {
        shared_ptr<BaseObject>& obj = elem.second;
        commandList->SetGraphicsRootDescriptorTable(0, obj->mTransform.GetRuntimeHandle().GetGPUHandle());
        for (auto component : obj->mComponents)
        {
            if (component->mComponentType == BaseComponentType::BASE_COMPONENT_MESH)
            {
                CoreMeshComponent* com = static_cast<CoreMeshComponent*>(&(*component));
                commandList->SetGraphicsRootDescriptorTable(2, com->mMat->GetRuntimeHandle().GetGPUHandle());

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
    Enqueue(mDrawContext->mCommandList.Get());
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