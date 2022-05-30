#include "CoreApp.h"

#include "../Core/Component/CoreMeshComponent.h"
#include "CoreGeometryUtils.h"
#include "../ThirdParty/ImGUI/imgui_internal.h"
#include "../ThirdParty/ImGUI/imgui.h"

CoreApp::CoreApp(HINSTANCE hInstance, BaseProject* proj) : BaseApp(hInstance, proj)
{
    // ..
    mUploadCmdList = std::make_unique<BaseDirectCommandList>(mDevice.Get());
    mUIDrawCmdList = std::make_unique<BaseDirectCommandList>(mDevice.Get());
    mLightManager = std::make_unique<CoreLightManager>(mDevice.Get());

    mLightManager->RegisterMainHandle(mMainHeap);
    
    mResourceManager = std::make_unique<CoreResourceManager>(mDevice.Get(), mProject->mAssetManager, mMainHeap);

    mGUIManager = std::make_unique<CoreGUIManager>(this, mResourceManager.get());

    mRenderTextureManager = std::make_unique<CoreRenderTextureManager>(mRuntimeHeap, mDevice.Get());
    mSceneRenderTexture = mRenderTextureManager->AllocateRenderTexture();
    mSceneRenderTexture->SetClearColor({ 0.1f, 0.1f, 0.3f, 1.0f });
    mEdgeRenderTexture = mRenderTextureManager->AllocateRenderTexture();
    mEdgeRenderTexture->SetClearColor({0.0f, 0.0f, 0.0f, 0.0f});

    // TBD: the generation of primitives should be moved in mesh loader
    std::shared_ptr<CoreGeometry> planeGeo = std::make_shared<CoreGeometry>(GetPlaneGeometry());
    mFullScreenPlane = std::make_unique<CoreMeshComponent>();
    mFullScreenPlane->mMeshType = CoreMeshComponentType::PRIMITIVE_COMPONENT;
    mFullScreenPlane->mID = "<plane>";
    mFullScreenPlane->AddGeometry(planeGeo);

    mMainCamera->SetRenderTexture(mSceneRenderTexture);

    // --- Draw Contexts ---

    // Diffuse Draw Context
    mDrawContext = std::make_unique<CoreDrawDiffuseContext>(mDevice.Get(), mProject->mAssetManager);

    // Edge Light Draw Context
    mEdgeLightDrawContext = std::make_unique<CoreDrawEdgeContext>(mDevice.Get(), mProject->mAssetManager);

    // Edge Light Blur Draw Context
    mBlurDrawContext = std::make_unique<CoreDrawBlurContext>(mDevice.Get(), mProject->mAssetManager);

    mMeshLoader = std::make_unique<CoreMeshLoader>();
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

  // Flush data from light component to light manager
  mLightManager->Update();

  // Register Runtime Heap
  mLightManager->RegisterRuntimeHandle(mRuntimeHeap);
  mGOManager->DispatchTransformUpload(mRuntimeHeap);
  mMainCamera->RegisterRuntimeHandle(mRuntimeHeap);
  mResourceManager->RegisterRuntimeHeap(mRuntimeHeap);

  // Upload newly-arrived geoemtry from CPU to GPU
  UploadGeometry();
  // Render object of interest's mask in to a texture for edge light
  RenderEdgeLightPre();
  // Render objects
  RenderObjects();
  // Render edge light with a blur of mask in `RenderEdgeLightPre`
  RenderBlurred();
  // Render the dear imGUI
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
        std::shared_ptr<BaseObject>& obj = elem.second;

        for (auto component : obj->mComponents)
        {
            if (component->mComponentType == ComponentType::COMPONENT_MESH)
            {
                CoreMeshComponent* com = dynamic_cast<CoreMeshComponent*>(&(*component));
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

    // Upload resources in reousrce Manager;
    mResourceManager->Upload(mUploadCmdList->mCommandList.Get());
    
    Enqueue(mUploadCmdList->mCommandList.Get());
}

void CoreApp::RenderBlurred()
{
    //mEdgeBlurredRenderTexture->SetWindow(mMainCamera->mScissorRect);

    mBlurDrawContext->ResetCommandList();
    ID3D12GraphicsCommandList* commandList = mBlurDrawContext->mCommandList.Get();

    commandList->SetGraphicsRootSignature(mBlurDrawContext->GetRootSig());

    commandList->RSSetViewports(1, &mMainCamera->mViewport);
    commandList->RSSetScissorRects(1, &mMainCamera->mScissorRect);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = mSceneRenderTexture->mRtvDescriptor;

    
    mSceneRenderTexture->BeginScene(commandList);
    commandList->OMSetRenderTargets(1, &rtv, false, nullptr);

    ID3D12DescriptorHeap* descriptorHeaps[] = { mRuntimeHeap->mDescHeap.Get() };
    commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    commandList->SetGraphicsRootDescriptorTable(0, mEdgeRenderTexture->mSRVHandle.GetGPUHandle());
    commandList->SetGraphicsRootDescriptorTable(1, mMainCamera->GetRuntimeHandle().GetGPUHandle());

    mFullScreenPlane->Render(commandList);
    
    mSceneRenderTexture->EndScene(commandList);
    Enqueue(mBlurDrawContext->mCommandList.Get());
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
        std::shared_ptr<BaseObject>& obj = elem.second;
        if (&*obj != mGUIManager->mNowSelectedObject)
        {
            continue;
        }
        
        commandList->SetGraphicsRootDescriptorTable(0, obj->mTransform.GetRuntimeHandle().GetGPUHandle());
        for (auto component : obj->mComponents)
        {
            if (component->mComponentType == ComponentType::COMPONENT_MESH)
            {     
                CoreMeshComponent* com = static_cast<CoreMeshComponent*>(&(*component));
                com->Render(commandList);
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
        std::shared_ptr<BaseObject>& obj = elem.second;
        commandList->SetGraphicsRootDescriptorTable(0, obj->mTransform.GetRuntimeHandle().GetGPUHandle());
        for (auto component : obj->mComponents)
        {
            if (component->mComponentType == ComponentType::COMPONENT_MESH)
            {
                CoreMeshComponent* com = static_cast<CoreMeshComponent*>(&(*component));

                for (int i = 0; i < com->mMesh.size(); ++i)
                {
                    auto& mesh = com->mMesh[i];
                    auto& mat = com->mMat[i];
                    auto& geo = com->mGeo[i];

                    if (mat)
                    {
                        commandList->SetGraphicsRootDescriptorTable(2, mat->mRuntimeHandle.GetGPUHandle());

                        if (mat->mStagedBuffer->mBuffer.mData.isBaseColorTextured)
                        {
                            BaseAssetNode* node = mProject->mAssetManager->LoadAssetByUUID(mat->mDiffuseColorTextureUUID);
                            BaseResourceImage* img = mResourceManager->LoadByURL<BaseResourceImage>(node->GetURL());

                            commandList->SetGraphicsRootDescriptorTable(4, img->mRuntimeHandle.GetGPUHandle());
                        }
                        
                        if (mat->mStagedBuffer->mBuffer.mData.isNormalTextured)
                        {
                            BaseAssetNode* node = mProject->mAssetManager->LoadAssetByUUID(mat->mNormalMapTextureUUID);
                            BaseResourceImage* img = mResourceManager->LoadByURL<BaseResourceImage>(node->GetURL());

                            commandList->SetGraphicsRootDescriptorTable(5, img->mRuntimeHandle.GetGPUHandle());
                        }
                    } else {
                        CoreResourceMaterial* defaultMat = mResourceManager->LoadByURL<CoreResourceMaterial>("EditorAsset\\Default.mat");
                        commandList->SetGraphicsRootDescriptorTable(2, defaultMat->mRuntimeHandle.GetGPUHandle());
                    }
                    

                    D3D12_INDEX_BUFFER_VIEW ibView = mesh->mIndexBufferView;
                    D3D12_VERTEX_BUFFER_VIEW vbView = mesh->mVertexBufferView;

                    commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                    commandList->IASetVertexBuffers(0, 1, &vbView);
                    commandList->IASetIndexBuffer(&ibView);

                    commandList->DrawIndexedInstanced(
                        static_cast<UINT>(geo->mIndices.size()),
                        1, 0, 0, 0
                    );
                }
            }
        }
    }

    mMainCamera->EndScene(commandList);
    Enqueue(mDrawContext->mCommandList.Get());
}