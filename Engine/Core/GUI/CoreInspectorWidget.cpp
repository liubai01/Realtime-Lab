#include "CoreInspectorWidget.h"
#include "../../Base/Component/BaseComponent.h"

#define PI 3.145926535897f

CoreInspectorWidget::CoreInspectorWidget(BaseObject** nowActiveObjectPtr, BaseResourceManager* resourceManager, BaseAssetManager* assetManager)
{
	mNowActiveObjectPtr = nowActiveObjectPtr;
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

	BaseObject* activeObj = *mNowActiveObjectPtr;

	if (activeObj)
	{
		char name[128]{ 0 };
		strcpy_s(name, activeObj->mName.c_str());

		if (ImGui::InputText("Name", name, IM_ARRAYSIZE(name), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			if (strlen(name) > 0)
			{
				activeObj->mName = std::string(name);
			}
		}

		UpdateTransformGUI(activeObj);


		for (std::shared_ptr<BaseComponent>& com : activeObj->mComponents)
		{
			com->OnEditorGUI(mAssetManager, mResourceManager);
		}

	}
	else {
		//ImGui::TextWrapped("Inspector shows details of selected object.");
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

		float rx = obj->mTransform.mRotation.x / PI * 180;
		float ry = obj->mTransform.mRotation.y / PI * 180;
		float rz = obj->mTransform.mRotation.z / PI * 180;

		ImGui::Text("X ");
		ImGui::SameLine();
		ImGui::DragFloat("rx", &rx, 0.5f, rx - 360, rx + 360, "%.02f deg.");

		ImGui::Text("Y ");
		ImGui::SameLine();
		ImGui::DragFloat("ry", &ry, 0.5f, ry - 360, ry + 360, "%.02f deg.");

		ImGui::Text("Z ");
		ImGui::SameLine();
		ImGui::DragFloat("rz", &rz, 0.5f, rz - 360, rz + 360, "%.02f deg.");

		obj->mTransform.SetRot(rx * PI / 180, ry * PI / 180, rz * PI / 180);


		ImGui::Text("");
		ImGui::Separator();
		ImGui::TreePop();
	}
}