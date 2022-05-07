#include "CoreGUIManager.h"
#include "../../ThirdParty/ImGUI/imgui_internal.h"
#include "../../ThirdParty/ImGUI/imgui_impl_dx12.h"

CoreGUIManager::CoreGUIManager(BaseApp* app)
{
    mApp = app;
    mWidgets.push_back(make_unique<CoreHierarchyWidget>(mApp->mGOManager));
    mFirstLoop = true;
}

void CoreGUIManager::Start()
{
    ImGuiID dockspace_id = ImGui::GetID("DockSpace");

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
}

void CoreGUIManager::Render(ID3D12GraphicsCommandList* commandList)
{
    // We are not required to set the viewports, the ImGUI would handle those kind of stuff
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = mApp->CurrentBackBufferView();
    commandList->OMSetRenderTargets(1, &rtv, false, nullptr);
    commandList->SetDescriptorHeaps(1, mApp->mUIRuntimeHeap->mDescHeap.GetAddressOf());

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}

void CoreGUIManager::Update()
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


        if (mFirstLoop)
        {
            Start();
            mFirstLoop = false;
        }

        for (unique_ptr<BaseGUIWidget>& widget : mWidgets)
        {
            widget->UpdateGUI();
        }

        ImGui::Begin("Scene");

            ImVec2 vMin = ImGui::GetWindowContentRegionMin();
            ImVec2 vMax = ImGui::GetWindowContentRegionMax();
            float height = vMax.y - vMin.y;
            float width = vMax.x - vMin.x;

            // set a minimal size of window
            height = max(height, 100);
            width = max(width, 100);

            mApp->mMainCamera->SetSize(width, height);
            ImGui::Image((ImTextureID)mApp->mMainCamera->mRTHandles[mApp->mFrameIdx].GetGPUHandle().ptr, ImVec2(width, height));

        ImGui::End(); // end of Scene

    ImGui::End(); // end of docker space
}