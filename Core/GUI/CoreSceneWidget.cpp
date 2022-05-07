#include "CoreSceneWidget.h"


CoreSceneWidget::CoreSceneWidget(BaseCamera* camera)
{
	mCamera = camera;
}

void CoreSceneWidget::Update()
{
    ImGui::Begin("Scene");

    ImVec2 vMin = ImGui::GetWindowContentRegionMin();
    ImVec2 vMax = ImGui::GetWindowContentRegionMax();
    float height = vMax.y - vMin.y;
    float width = vMax.x - vMin.x;

    // set a minimal size of window
    height = max(height, 100);
    width = max(width, 100);

    mCamera->SetSize(width, height);
    ImGui::Image((ImTextureID) (mCamera->GetRenderTextureHandle()).GetGPUHandle().ptr, ImVec2(width, height));

    ImGui::End(); // end of Scene
}

void CoreSceneWidget::Start(ImGuiID& dockspace_id)
{
    ImGui::DockBuilderDockWindow("Scene", dockspace_id);
}