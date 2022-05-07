#include "CoreHierarchyWidget.h"
#include <stack>

using namespace std;

CoreHierarchyWidget::CoreHierarchyWidget(BaseGameObjectManager* GOManager, BaseObject** nowSelectObjectPtr)
{
	mGOManager = GOManager;
	mNowSelectObjectPtr = nowSelectObjectPtr;
}

void DFSTreeNode(BaseObject* obj, BaseObject** nowSelectObjectPtr)
{
	bool hasChild = obj->GetChildObjects().size() != 0;

	if (hasChild)
	{
		auto treeNode = ImGui::TreeNodeEx(obj->mName.c_str(), ImGuiTreeNodeFlags_DefaultOpen, "");
		ImGui::SameLine();
		if (ImGui::SmallButton(obj->mName.c_str()))
		{
			*nowSelectObjectPtr = obj;
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
		ImGui::BulletText("");
		ImGui::SameLine();
		if (ImGui::SmallButton(obj->mName.c_str()))
		{
			*nowSelectObjectPtr = obj;
		}
	}
}

void CoreHierarchyWidget::Update()
{
	ImGui::Begin("Hierarchy");
	for (BaseObject* obj : mGOManager->mRootObjects)
	{
		DFSTreeNode(obj, mNowSelectObjectPtr);
	}
	ImGui::End();
}

void CoreHierarchyWidget::Start(ImGuiID& dockspace_id)
{
	ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dockspace_id);
	ImGui::DockBuilderDockWindow("Hierarchy", dock_left_id);
}