#include "CoreHierarchyWidget.h"
#include <stack>

CoreHierarchyWidget::CoreHierarchyWidget(BaseGameObjectManager* GOManager, BaseObject** nowSelectObjectPtr)
{
	mIsShow = true;
	mDisplayName = "Hierarchy";
	mGOManager = GOManager;
	mNowSelectObjectPtr = nowSelectObjectPtr;
}

void DFSTreeNode(BaseObject* obj, BaseObject** nowSelectObjectPtr)
{
	bool hasChild = obj->GetChildObjects().size() != 0;

	if (hasChild)
	{
		// check whether current object is selected
		int flag = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;
		if (obj == *nowSelectObjectPtr)
		{
			flag = flag | ImGuiTreeNodeFlags_Selected;
		}
		auto treeNode = ImGui::TreeNodeEx((obj->mName + "treenode").c_str(), flag, "");
		ImGui::SameLine();
		bool button = ImGui::SmallButton(obj->mName.c_str());

		if (button)
		{
			*nowSelectObjectPtr = obj;
		}

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			ImGui::SetDragDropPayload("GAMEOBJECT", obj, sizeof(BaseObject*));

			ImGui::Text(("<GameObject> " + obj->mName).c_str());
			ImGui::EndDragDropSource();
		}


		if (treeNode)
		{
			for (BaseObject* childobj : obj->GetChildObjects())
			{
				DFSTreeNode(childobj, nowSelectObjectPtr);
			}
			ImGui::TreePop();
		}
	} else {
		int flag = ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
		if (obj == *nowSelectObjectPtr)
		{
			flag = flag | ImGuiTreeNodeFlags_Selected;
		}
		// avoid name collision with button, add a tree postfix
		ImGui::TreeNodeEx((obj->mName + "tree").c_str(), flag, "");

		ImGui::SameLine();
		if (ImGui::SmallButton(obj->mName.c_str()))
		{
			*nowSelectObjectPtr = obj;
		}

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			ImGui::SetDragDropPayload("GAMEOBJECT", obj, sizeof(BaseObject*));

			ImGui::Text(("<GameObject> " + obj->mName).c_str());
			ImGui::EndDragDropSource();
		}

	}
}

void CoreHierarchyWidget::Update()
{
	ImGui::Begin(mDisplayName.c_str());

	ImGuiIO& io = ImGui::GetIO();
	int count = IM_ARRAYSIZE(io.MouseDown);

	ImVec2 vMin = ImGui::GetWindowContentRegionMin();
	ImVec2 vMax = ImGui::GetWindowContentRegionMax();

	vMin.x += ImGui::GetWindowPos().x;
	vMin.y += ImGui::GetWindowPos().y;
	vMax.x += ImGui::GetWindowPos().x;
	vMax.y += ImGui::GetWindowPos().y;

	// if left click in hierarchy menu, reset mNowSelectObjectPtr by default
	if (ImGui::IsMouseReleased(0))
	{
		if (io.MousePos.x > vMin.x && io.MousePos.x < vMax.x && io.MousePos.y > vMin.y && io.MousePos.y < vMax.y)
		{
			*mNowSelectObjectPtr = nullptr;
		}
	}

	for (BaseObject* obj : mGOManager->mRootObjects)
	{
		DFSTreeNode(obj, mNowSelectObjectPtr);
	}
	ImGui::End();
}

void CoreHierarchyWidget::Start(ImGuiID& dockspace_id)
{
	ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dockspace_id);
	ImGui::DockBuilderDockWindow(mDisplayName.c_str(), dock_left_id);
}