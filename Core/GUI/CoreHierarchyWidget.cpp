#include "CoreHierarchyWidget.h"
#include <stack>

using namespace std;

CoreHierarchyWidget::CoreHierarchyWidget(BaseGameObjectManager* GOManager)
{
	mGOManager = GOManager;
}

void DFSTreeNode(BaseObject* obj)
{
	bool hasChild = obj->GetChildObjects().size() != 0;

	if (hasChild)
	{
		if (ImGui::TreeNode(obj->mName.c_str()))
		{
			for (BaseObject* childobj : obj->GetChildObjects())
			{
				DFSTreeNode(childobj);
			}
			ImGui::TreePop();
		}
	} else {
		ImGui::BulletText(obj->mName.c_str());
	}
}

void CoreHierarchyWidget::UpdateGUI()
{
	ImGui::Begin("Hierarchy");
	for (BaseObject* obj : mGOManager->mRootObjects)
	{
		DFSTreeNode(obj);
	}
	ImGui::End();
}