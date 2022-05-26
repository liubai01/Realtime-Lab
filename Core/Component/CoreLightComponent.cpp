#include "CoreLightComponent.h"
#include "../../ThirdParty/ImGUI/imgui.h"
#include <math.h>
#include "../../DebugOut.h"

CoreLightComponent::CoreLightComponent()
{
	mComponentType = ComponentType::COMPONENT_LIGHT;

	// Default light setup
	mData.Ims = { 0.5f, 0.5f, 0.5f, 1.0f };
	mData.Ia = { 0.0f, 0.0f, 0.0f, 1.0f };
	mData.Id = { 1.0f, 1.0f, 1.0f, 1.0f };
	mData.LightDir = { 0.577f, 0.577f, 0.577f, 0.0f };

	mIdMag = 1.0f;
	mDiffuseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
}

void CoreLightComponent::SetLightDir(float x, float y, float z)
{
	mData.LightDir = { x, y, z, 0.0f };
}

void CoreLightComponent::OnEditorGUI()
{
	if (ImGui::TreeNodeEx("Light Component", ImGuiTreeNodeFlags_Framed))
	{
		
		ImGui::Text("Light Color");
		ImGui::ColorEdit3("LightDiffuseColor##", (float*) &mDiffuseColor, ImGuiColorEditFlags_NoLabel);

		ImGui::NewLine();
		ImGui::Text("Light Intensity");
		ImGui::DragFloat("LightIntensity##", &mIdMag, 0.01f, 0.00f, 10.0f, "%03.02f");
		
		mData.Id.x = mIdMag * mDiffuseColor.x;
		mData.Id.y = mIdMag * mDiffuseColor.y;
		mData.Id.z = mIdMag * mDiffuseColor.z;

		ImGui::Separator();
		ImGui::TreePop();
	}
}

