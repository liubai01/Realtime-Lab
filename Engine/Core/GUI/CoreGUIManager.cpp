#include "CoreGUIManager.h"
#include "CoreSceneWidget.h"
#include "CoreInspectorWidget.h"
#include "CoreAssetWidget.h"
#include "../../ThirdParty/ImGUI/imgui_internal.h"
#include "../../ThirdParty/ImGUI/imgui_impl_dx12.h"

CoreGUIManager::CoreGUIManager(BaseApp* app, CoreResourceManager* resourceManager)
{
    mApp = app;
    mWidgets.push_back(std::make_unique<CoreAssetWidget>(mApp->mProject->mAssetManager, resourceManager));
    mWidgets.push_back(std::make_unique<CoreHierarchyWidget>(mApp->mGOManager, &mNowSelectedObject));
    mWidgets.push_back(std::make_unique<CoreSceneWidget>(mApp->mMainCamera));
    mWidgets.push_back(std::make_unique<CoreInspectorWidget>(&mNowSelectedObject, resourceManager, app->mProject->mAssetManager));
    
    mFirstLoop = true;
    mNowSelectedObject = nullptr;

    mResourceManager = resourceManager;

    ImGuiIO& io = ImGui::GetIO();
    BaseAssetNode* fontNode = app->mProject->mAssetManager->LoadAsset("EditorAsset\\Cousine-Regular.ttf");
    io.Fonts->AddFontFromFileTTF(fontNode->mFullPath.c_str(), 16);
}

void CoreGUIManager::Start()
{
    ImGuiID dockspace_id = ImGui::GetID("DockSpace");

    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);

    // Make the dock node's size and position to match the viewport
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);
    ImGui::DockBuilderSetNodePos(dockspace_id, ImGui::GetMainViewport()->WorkPos);

    for (std::unique_ptr<BaseGUIWidget>& widget : mWidgets)
    {
        widget->Start(dockspace_id);
    }
    
    ImGui::DockBuilderFinish(dockspace_id);
}

void CoreGUIManager::Render(ID3D12GraphicsCommandList* commandList)
{
    // We are not required to set the viewports, the ImGUI would handle those kind of stuff
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = mApp->CurrentBackBufferView();
    commandList->OMSetRenderTargets(1, &rtv, false, nullptr);
    commandList->SetDescriptorHeaps(1, mApp->mRuntimeHeap->mDescHeap.GetAddressOf());

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
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

        for (std::unique_ptr<BaseGUIWidget>& widget : mWidgets)
        {
            widget->Update();
        }
    
    ImGui::End(); // end of docker space


}