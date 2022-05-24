#include "CoreApp.h"

#include "../Core/Component/CoreMeshComponent.h"
#include "CoreGeometryUtils.h"
#include "../ThirdParty/ImGUI/imgui_internal.h"
#include "../ThirdParty/ImGUI/imgui.h"

CoreApp::CoreApp(HINSTANCE hInstance, const string& ProjectPath) : BaseApp(hInstance, ProjectPath)
{
    mUploadCmdList = make_unique<BaseDirectCommandList>(mDevice.Get());
    mUIDrawCmdList = make_unique<BaseDirectCommandList>(mDevice.Get());

    mMaterialManager = make_unique<CoreMaterialManager>(mDevice.Get(), mMainHeap);
    mLightManager = make_unique<CoreLightManager>(mDevice.Get());

    mLightManager->RegisterMainHandle(mMainHeap);
    mGUIManager = make_unique<CoreGUIManager>(this);

    mRenderTextureManager = make_unique<CoreRenderTextureManager>(mRuntimeHeap, mDevice.Get());
    mSceneRenderTexture = mRenderTextureManager->AllocateRenderTexture();
    mSceneRenderTexture->SetClearColor({ 0.1f, 0.1f, 0.3f, 1.0f });
    mEdgeRenderTexture = mRenderTextureManager->AllocateRenderTexture();
    mEdgeRenderTexture->SetClearColor({0.0f, 0.0f, 0.0f, 0.0f});

    shared_ptr<CoreGeometry> planeGeo = make_shared<CoreGeometry>(GetPlaneGeometry());
    mFullScreenPlane = make_unique<CoreMeshComponent>();
    mFullScreenPlane->AddGeometry(planeGeo);

    mMainCamera->SetRenderTexture(mSceneRenderTexture);

    // Diffuse Draw Context
    mDrawContext = make_unique<CoreDrawDiffuseContext>(mDevice.Get());

    // Edge Light Draw Context
    mEdgeLightDrawContext = make_unique<CoreDrawEdgeContext>(mDevice.Get());

    // Edge Light Blur Draw Context
    mBlurDrawContext = make_unique<CoreDrawBlurContext>(mDevice.Get());

    mMeshLoader = make_unique<CoreMeshLoader>(&*mMaterialManager);
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
        shared_ptr<BaseObject>& obj = elem.second;
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
        shared_ptr<BaseObject>& obj = elem.second;
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

                    commandList->SetGraphicsRootDescriptorTable(2, mat->GetRuntimeHandle().GetGPUHandle());

                    if (mat->mBuffer.mData.isBaseColorTextured)
                    {
                        commandList->SetGraphicsRootDescriptorTable(4, mat->mDiffuseColorTexture->mRuntimeHandle.GetGPUHandle());
                    }
                    
                    if (mat->mBuffer.mData.isNormalTextured)
                    {
                        commandList->SetGraphicsRootDescriptorTable(5, mat->mNormalMapTexture->mRuntimeHandle.GetGPUHandle());
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