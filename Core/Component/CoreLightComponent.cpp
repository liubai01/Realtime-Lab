#include "CoreLightComponent.h"
#include "../../ThirdParty/ImGUI/imgui.h"


CoreLightComponent::CoreLightComponent()
{
	mComponentType = ComponentType::COMPONENT_LIGHT;

	// Default light setup
	mData.Ims = { 0.5f, 0.5f, 0.5f, 1.0f };
	mData.Ia = { 0.2f, 0.2f, 0.2f, 1.0f };
	mData.Id = { 1.0f, 1.0f, 1.0f, 1.0f };
	mData.LightDir = { 0.577f, 0.577f, 0.577f, 0.0f };
}

void CoreLightComponent::SetLightDir(float x, float y, float z)
{
	mData.LightDir = { x, y, z, 0.0f };
}

void CoreLightComponent::OnEditorGUI()
{
	if (ImGui::TreeNodeEx("Light Component", ImGuiTreeNodeFlags_DefaultOpen))
	{

		ImGui::Separator();
		ImGui::TreePop();
	}
}

