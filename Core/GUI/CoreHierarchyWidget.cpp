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
		// check whether current object is selected
		int flag = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;
		if (obj == *nowSelectObjectPtr)
		{
			flag = flag | ImGuiTreeNodeFlags_Selected;
		}
		auto treeNode = ImGui::TreeNodeEx(obj->mName.c_str(), flag, "");
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
	}
}

void CoreHierarchyWidget::Update()
{
	ImGui::Begin("Hierarchy");

	ImGui::Text("Keys down:");
	const ImGuiKey key_first = 0;
	struct funcs { 
		static bool IsLegacyNativeDupe(ImGuiKey key) 
		{ 
			return key < 512 && ImGui::GetIO().KeyMap[key] != -1; 
		} 
	}; // Hide Native<>ImGuiKey duplicates when both exists in the array

	for (ImGuiKey key = key_first; key < ImGuiKey_COUNT; key++) 
	{ 
		if (funcs::IsLegacyNativeDupe(key)) continue; 
		if (ImGui::IsKeyDown(key)) 
		{ 
			ImGui::SameLine(); 
			ImGui::Text("\"%s\" %d (%.02f secs)", ImGui::GetKeyName(key), key, ImGui::GetKeyData(key)->DownDuration); 
		} 
	}
	/*ImGui::Text("Keys pressed:");       for (ImGuiKey key = key_first; key < ImGuiKey_COUNT; key++) { if (funcs::IsLegacyNativeDupe(key)) continue; if (ImGui::IsKeyPressed(key)) { ImGui::SameLine(); ImGui::Text("\"%s\" %d", ImGui::GetKeyName(key), key); } }
	ImGui::Text("Keys released:");      for (ImGuiKey key = key_first; key < ImGuiKey_COUNT; key++) { if (funcs::IsLegacyNativeDupe(key)) continue; if (ImGui::IsKeyReleased(key)) { ImGui::SameLine(); ImGui::Text("\"%s\" %d", ImGui::GetKeyName(key), key); } }*/

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