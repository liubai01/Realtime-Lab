#include "CoreLoggerWidget.h"

CoreLoggerWidget::CoreLoggerWidget()
{
    mIsShow = false;
    mDisplayName = "Logger";
}


void CoreLoggerWidget::Update()
{
    ImGui::Begin(mDisplayName.c_str());

    ImGui::Text("Hello");

    ImGui::End(); // end of Scene
}

void CoreLoggerWidget::Start(ImGuiID& dockspace_id)
{
    ImGui::DockBuilderDockWindow(mDisplayName.c_str(), dockspace_id);
}