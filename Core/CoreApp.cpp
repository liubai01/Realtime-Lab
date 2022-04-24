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
  RenderUI();

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
    BaseRenderTexture* nowRT = &*mMainCamera->mRenderTextures[mFrameIdx];
    ID3D12GraphicsCommandList* commandList = mDrawContext->mCommandList.Get();

    nowRT->BeginScene(commandList);

    commandList->SetGraphicsRootSignature(mDrawContext->GetRootSig());
    commandList->RSSetViewports(1, &mMainCamera->mViewport);
    commandList->RSSetScissorRects(1, &mMainCamera->mScissorRect);

    // Clear the back buffer and depth buffer.
    //D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = nowRT->m_rtvDescriptor;
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

    nowRT->EndScene(commandList);
}

void CoreApp::RenderUI()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("About"))
        {
            if (ImGui::MenuItem("@liubai01")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    static bool firstLoop = true;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoDocking;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Invisible Docker Space", nullptr, window_flags);

    ImGui::PopStyleVar(3);

    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    ImGuiID dockspace_id = ImGui::GetID("DockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);


    if (firstLoop)
    {
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);

        // Make the dock node's size and position to match the viewport
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);
        ImGui::DockBuilderSetNodePos(dockspace_id, ImGui::GetMainViewport()->WorkPos);

        //ImGuiID dock_main_id = mainNodeID;
        ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dockspace_id);
        ImGui::DockBuilderDockWindow("Hierarchy", dock_left_id);
        ImGui::DockBuilderDockWindow("Scene", dockspace_id);

        ImGui::DockBuilderFinish(dockspace_id);

        firstLoop = false;
    }

    ImGui::Begin("Hierarchy");
    ImGui::Text("Hierarchy TODO here");
    ImGui::End();

    ImGui::Begin("Scene");
    ImGui::Text("Scene TODO here");
    
    BaseRenderTexture* nowRT = &*mMainCamera->mRenderTextures[mFrameIdx];
    ImGui::Image((ImTextureID) mMainCamera->mRTHandles[mFrameIdx].GetGPUHandle().ptr, ImVec2(nowRT->m_width / 2.0, nowRT->m_height / 2.0));
    ImGui::End();


    ImGui::End();

    ID3D12GraphicsCommandList* commandList = mDrawContext->mCommandList.Get();

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    commandList->OMSetRenderTargets(1, &rtv, false, nullptr);

    commandList->SetDescriptorHeaps(1, mUIRuntimeHeap->mDescHeap.GetAddressOf());

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
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