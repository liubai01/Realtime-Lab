#include "CoreSceneWidget.h"


CoreSceneWidget::CoreSceneWidget(BaseCamera* camera)
{
    mIsShow = true;
    mDisplayName = "Scene";
	mCamera = camera;
}

void CoreSceneWidget::Update()
{
    ImGui::Begin(mDisplayName.c_str());

    ImVec2 vMin = ImGui::GetWindowContentRegionMin();
    ImVec2 vMax = ImGui::GetWindowContentRegionMax();
    float height = vMax.y - vMin.y;
    float width = vMax.x - vMin.x;

    // set a minimal size of window
    height = max(height, 100);
    width = max(width, 100);

    mCamera->SetSize(width, height);
    ImGui::Image((ImTextureID) (mCamera->GetRenderTextureSRVHandle()).GetGPUHandle().ptr, ImVec2(width, height));

    ImGui::End(); // end of Scene
}

void CoreSceneWidget::Start(ImGuiID& dockspace_id)
{
    ImGui::DockBuilderDockWindow(mDisplayName.c_str(), dockspace_id);
}