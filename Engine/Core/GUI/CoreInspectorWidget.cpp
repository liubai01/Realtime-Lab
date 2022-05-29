#include "CoreInspectorWidget.h"
#include "../../Base/Component/BaseComponent.h"

#define PI 3.145926535897f

CoreInspectorWidget::CoreInspectorWidget(BaseObject** nowSelectObjectPtr, BaseResourceManager* resourceManager, BaseAssetManager* assetManager)
{
	mNowSelectObjectPtr = nowSelectObjectPtr;
	mResourceManager = resourceManager;
	mAssetManager = assetManager;
}

void CoreInspectorWidget::Start(ImGuiID& dockspace_id)
{
	ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.25f, nullptr, &dockspace_id);
	ImGui::DockBuilderDockWindow("Inspector", dock_right_id);
}
void CoreInspectorWidget::Update()
{

	ImGui::Begin("Inspector");

	BaseObject* obj = *mNowSelectObjectPtr;
	if (obj)
	{
		char name[128]{ 0 };
		strcpy_s(name, obj->mName.c_str());

		if (ImGui::InputText("Name", name, IM_ARRAYSIZE(name), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			if (strlen(name) > 0)
			{
				obj->mName = std::string(name);
			}
		}

		UpdateTransformGUI(obj);


		for (std::shared_ptr<BaseComponent>& com : obj->mComponents)
		{
			com->OnEditorGUI(mAssetManager, mResourceManager);
		}

	}
	else {
		ImGui::TextWrapped("Inspector shows details of selected object.");
	}




	ImGui::End();
}

void CoreInspectorWidget::UpdateTransformGUI(BaseObject* obj)
{
	ImGui::Separator();

	if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("");
		ImGui::Text("Position");
		ImGui::Text("X ");
		ImGui::SameLine();
		ImGui::DragFloat("px", &obj->mTransform.mPosition.x, 0.01f, obj->mTransform.mPosition.x - 10.0f, obj->mTransform.mPosition.x + 10.0f, "%.02f");

		ImGui::Text("Y ");
		ImGui::SameLine();
		ImGui::DragFloat("py", &obj->mTransform.mPosition.y, 0.01f, obj->mTransform.mPosition.y - 10.0f, obj->mTransform.mPosition.y + 10.0f, "%.02f");

		ImGui::Text("Z ");
		ImGui::SameLine();
		ImGui::DragFloat("pz", &obj->mTransform.mPosition.z, 0.01f, obj->mTransform.mPosition.z - 10.0f, obj->mTransform.mPosition.z + 10.0f, "%.02f");
		ImGui::Text("");

		ImGui::Text("Scale");
		ImGui::Text("X ");
		ImGui::SameLine();
		ImGui::DragFloat("sx", &obj->mTransform.mScale.x, 0.01f, obj->mTransform.mScale.x - 10.0f, obj->mTransform.mScale.x + 10.0f, "%.02f");
		ImGui::Text("Y ");
		ImGui::SameLine();
		ImGui::DragFloat("sy", &obj->mTransform.mScale.y, 0.01f, obj->mTransform.mScale.y - 10.0f, obj->mTransform.mScale.y + 10.0f, "%.02f");
		ImGui::Text("Z ");
		ImGui::SameLine();
		ImGui::DragFloat("sz", &obj->mTransform.mScale.z, 0.01f, obj->mTransform.mScale.z - 10.0f, obj->mTransform.mScale.z + 10.0f, "%03.02f");
		ImGui::Text("");

		ImGui::Text("Rotation (Eular)");

		float rx = obj->mTransform.mRotation.x / PI * 360;
		float ry = obj->mTransform.mRotation.y / PI * 360;
		float rz = obj->mTransform.mRotation.z / PI * 360;

		ImGui::Text("X ");
		ImGui::SameLine();
		ImGui::DragFloat("rx", &rx, 0.5f, rx - 360, rx + 360, "%.02f deg.");

		ImGui::Text("Y ");
		ImGui::SameLine();
		ImGui::DragFloat("ry", &ry, 0.5f, ry - 360, ry + 360, "%.02f deg.");

		ImGui::Text("Z ");
		ImGui::SameLine();
		ImGui::DragFloat("rz", &rz, 0.5f, rz - 360, rz + 360, "%.02f deg.");

		obj->mTransform.SetRot(rx * PI / 360, ry * PI / 360, rz * PI / 360);


		ImGui::Text("");
		ImGui::Separator();
		ImGui::TreePop();
	}
}