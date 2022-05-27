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

	mLightIntensity = 1.0f;
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
		ImGui::DragFloat("LightIntensity##", &mLightIntensity, 0.01f, 0.00f, 10.0f, "%03.02f");
		
		mData.Id.x = mLightIntensity * mDiffuseColor.x;
		mData.Id.y = mLightIntensity * mDiffuseColor.y;
		mData.Id.z = mLightIntensity * mDiffuseColor.z;

		ImGui::Separator();
		ImGui::TreePop();
	}
}

json CoreLightComponent::Serialize()
{
	json j = json{ 
		{"lightIntensity", mLightIntensity},
		{"diffuseColor", {mDiffuseColor.x, mDiffuseColor.y, mDiffuseColor.z} }
	};
	return j;
}

void CoreLightComponent::Deserialize(const json& j)
{
	j.at("lightIntensity").get_to(mLightIntensity);
	j.at("diffuseColor")[0].get_to(mDiffuseColor.x);
	j.at("diffuseColor")[1].get_to(mDiffuseColor.y);
	j.at("diffuseColor")[2].get_to(mDiffuseColor.z);
}

