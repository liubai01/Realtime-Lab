#include "CoreAssetWidget.h"


CoreAssetWidget::CoreAssetWidget(BaseAssetManager* assetManager, BaseResourceManager* resourceManager)
{
	mAssetManager = assetManager;
	mResourceManager = resourceManager;
	mNowSelectedAssetNodeDir = mAssetManager->mRootAsset.get();
}

void CoreAssetWidget::Update()
{
	// Explorer window
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0);
	ImGui::Begin("Asset");
		ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		ImVec2 vMax = ImGui::GetWindowContentRegionMax();

		static float w = 200.0f;
		static float h = vMax.y - vMin.y;


		ImGui::BeginChild("child1", ImVec2(w, h), true);
		UpdateAssetExplorerWindow();
		ImGui::EndChild();
		ImGui::SameLine();

		// vertex splitter
		ImGui::InvisibleButton("vsplitter", ImVec2(6.0f, h));
		if (ImGui::IsItemActive())
			w += ImGui::GetIO().MouseDelta.x;
		ImGui::SameLine();

		// Content window
		ImGui::BeginChild("child2", ImVec2(0, h), true);
		UpdateAssetContentWindow();
		ImGui::EndChild();

	ImGui::End();
	ImGui::PopStyleVar(1);
}

void CoreAssetWidget::UpdateAssetContentWindow()
{
	if (mNowSelectedAssetNodeDir)
	{
		for (unique_ptr<BaseAssetNode>& assetNode : mNowSelectedAssetNodeDir->mSubAssets)
		{
			if (assetNode->IsHidden()) 
			{
				continue;
			}

			string displayName;
			static float iconSize = 120;
			static float nameTabHeight = 20;

			// show part of the name if there is no enough space
			static size_t showCharNum = 10;
			displayName = assetNode->mID;
			if (displayName.size() > showCharNum)
			{
				displayName = "..." + displayName.substr(displayName.size() - showCharNum);
			}
			
			ImGui::BeginGroup();
			if (assetNode->GetAssetType() == BaseAssetType::ASSET_IMAGE)
			{
				BaseResourceImage* img = mResourceManager->LoadByURL<BaseResourceImage>(assetNode->GetRelativePath());
				ImGui::Image(img->GetImGuiRuntimeID(), ImVec2(iconSize, iconSize));

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
				{
					BaseAssetNode* ptr = assetNode.get();
					ImGui::SetDragDropPayload("ASSET_CELL", &ptr, sizeof(BaseAssetNode*));

					ImGui::Image(img->GetImGuiRuntimeID(), ImVec2(iconSize, iconSize));
					ImGui::Text(assetNode->mID.c_str());
					ImGui::EndDragDropSource();
				}
			} else if (assetNode->GetAssetType() == BaseAssetType::ASSET_FOLDER) {
				ImVec2 pos = ImGui::GetCursorPos();
				if (ImGui::Selectable(assetNode->mID.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(iconSize, iconSize)))
				{
					ImGuiIO& io = ImGui::GetIO();
					if (io.MouseDoubleClicked[0])
					{
						mNowSelectedAssetNodeDir = assetNode.get();
					}
				}

				ImGui::SetCursorPos(ImVec2(pos.x + 20.0f, pos.y + 20.0f));
				BaseResourceImage* img = mResourceManager->LoadByURL<BaseResourceImage>("EditorAsset\\icon\\folder.png");
				ImGui::Image(img->GetImGuiRuntimeID(), ImVec2(iconSize - 40.0f, iconSize - 40.0f));

				ImGui::SetCursorPos(ImVec2(pos.x, pos.y + iconSize));

			} else {
				ImGui::Button(assetNode->mID.c_str(), ImVec2(iconSize, iconSize));
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
				{
					BaseAssetNode* ptr = assetNode.get();
					ImGui::SetDragDropPayload("ASSET_CELL", &ptr, sizeof(BaseAssetNode*));

					ImGui::Text(assetNode->mID.c_str());
					ImGui::EndDragDropSource();
				}
			}



			ImGui::Button(displayName.c_str(), ImVec2(iconSize, nameTabHeight));

			ImGui::EndGroup();

			ImGui::SameLine();


		}
	}
}

void CoreAssetWidget::DFSConstructExploererNode(BaseAssetNode* nowNode)
{
	bool hasChild;
	int cnt = 0;
	for (unique_ptr<BaseAssetNode>& assetNode : nowNode->mSubAssets)
	{
		if (assetNode->GetAssetType() == BaseAssetType::ASSET_FOLDER && !assetNode->IsHidden())
		{
			++cnt;
		}
	}
	hasChild = cnt != 0;

	if (hasChild)
	{
		// check whether current object is selected
		int flag = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;
		if (nowNode == mNowSelectedAssetNodeDir)
		{
			flag = flag | ImGuiTreeNodeFlags_Selected;
		}
		auto treeNode = ImGui::TreeNodeEx(nowNode->mID.c_str(), flag, "");
		ImGui::SameLine();
		if (ImGui::SmallButton(nowNode->mID.c_str()))
		{
			mNowSelectedAssetNodeDir = nowNode;
		}

		if (treeNode)
		{
			for (unique_ptr<BaseAssetNode>& assetNode :nowNode->mSubAssets)
			{
				if (assetNode->GetAssetType() == BaseAssetType::ASSET_FOLDER && !assetNode->IsHidden())
				{
					DFSConstructExploererNode(assetNode.get());
				}
			}
			ImGui::TreePop();
		}
	}
	else {
		int flag = ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
		if (nowNode == mNowSelectedAssetNodeDir)
		{
			flag = flag | ImGuiTreeNodeFlags_Selected;
		}
		// avoid name collision with button, add a tree postfix
		ImGui::TreeNodeEx((nowNode->mID + "tree").c_str(), flag, "");

		ImGui::SameLine();
		if (ImGui::SmallButton(nowNode->mID.c_str()))
		{
			mNowSelectedAssetNodeDir = nowNode;
		}
	}
}


void CoreAssetWidget::UpdateAssetExplorerWindow()
{
	ImGuiIO& io = ImGui::GetIO();
	int count = IM_ARRAYSIZE(io.MouseDown);

	ImVec2 vMin = ImGui::GetWindowContentRegionMin();
	ImVec2 vMax = ImGui::GetWindowContentRegionMax();

	vMin.x += ImGui::GetWindowPos().x;
	vMin.y += ImGui::GetWindowPos().y;
	vMax.x += ImGui::GetWindowPos().x;
	vMax.y += ImGui::GetWindowPos().y;

	// if left click in hierarchy menu, reset mNowSelectObjectPtr by default
	if (ImGui::IsMouseClicked(0))
	{
		if (io.MousePos.x > vMin.x && io.MousePos.x < vMax.x && io.MousePos.y > vMin.y && io.MousePos.y < vMax.y)
		{
			mNowSelectedAssetNodeDir = nullptr;
		}
	}

	DFSConstructExploererNode(mAssetManager->mRootAsset.get());
}

void CoreAssetWidget::Start(ImGuiID& dockspace_id)
{
	ImGuiID dock_down_id = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.25f, nullptr, &dockspace_id);

	ImGui::DockBuilderDockWindow("Asset", dock_down_id);
}