#include "CoreAssetWidget.h"
#include "../../Base/Util/FileOpenUtils.h"

CoreAssetWidget::CoreAssetWidget(BaseAssetManager* assetManager, BaseResourceManager* resourceManager)
{
	mIsShow = true;
	mDisplayName = "Asset Explorer";

	mAssetManager = assetManager;
	mResourceManager = resourceManager;
	mNowSelectedAssetNodeDir = mAssetManager->mRootAsset.get();
}

template<typename T>
void ifLeftClickCurrentWindow(T&& lambda) {
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
			lambda();
		}
	}
}

template<typename T>
void ifRightClickCurrentWindow(T&& lambda) {
	ImGuiIO& io = ImGui::GetIO();
	int count = IM_ARRAYSIZE(io.MouseDown);

	ImVec2 vMin = ImGui::GetWindowContentRegionMin();
	ImVec2 vMax = ImGui::GetWindowContentRegionMax();

	vMin.x += ImGui::GetWindowPos().x;
	vMin.y += ImGui::GetWindowPos().y;
	vMax.x += ImGui::GetWindowPos().x;
	vMax.y += ImGui::GetWindowPos().y;

	// if left click in hierarchy menu, reset mNowSelectObjectPtr by default
	if (ImGui::IsMouseClicked(1))
	{
		if (io.MousePos.x > vMin.x && io.MousePos.x < vMax.x && io.MousePos.y > vMin.y && io.MousePos.y < vMax.y)
		{
			lambda();
		}
	}
}

void CoreAssetWidget::Update()
{
	// Explorer window
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0);
	ImGui::Begin(mDisplayName.c_str());
		ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		ImVec2 vMax = ImGui::GetWindowContentRegionMax();

		static float w = 200.0f;
		static float h = vMax.y - vMin.y - 10.0f;


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
	ImVec2 vMin = ImGui::GetWindowContentRegionMin();
	ImVec2 vMax = ImGui::GetWindowContentRegionMax();
	float posX = 0.0f;

	static float iconSize = 120;
	static float nameTabHeight = 20;

	if (mNowSelectedAssetNodeDir)
	{
		for (std::unique_ptr<BaseAssetNode>& assetNode : mNowSelectedAssetNodeDir->mSubAssets)
		{
			if (assetNode->IsHidden()) 
			{
				continue;
			}
			std::string displayName;

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
				BaseResourceImage* img = mResourceManager->LoadByURL<BaseResourceImage>(assetNode->GetURL());
				ImTextureID imgID = img->GetImGuiRuntimeID();

				if (imgID)
				{
					ImGui::Image(imgID, ImVec2(iconSize, iconSize));
				} else {
					ImGui::Selectable("Loading...", false, ImGuiSelectableFlags_None,ImVec2(iconSize, iconSize));
				}
				

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

				// manually set a padding of 20.0 pixel (TBD: remove this magic number in the future)
				ImGui::SetCursorPos(ImVec2(pos.x + 20.0f, pos.y + 20.0f));
				BaseResourceImage* img = mResourceManager->LoadByURL<BaseResourceImage>("EditorAsset\\icon\\folder.png");
				ImGui::Image(img->GetImGuiRuntimeID(), ImVec2(iconSize - 40.0f, iconSize - 40.0f));

				ImGui::SetCursorPos(ImVec2(pos.x, pos.y + iconSize));
			}
			else if (assetNode->GetAssetType() == BaseAssetType::ASSET_MATERIAL) {
				ImVec2 pos = ImGui::GetCursorPos();
				if (ImGui::Selectable(assetNode->mID.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(iconSize, iconSize)))
				{
					ImGuiIO& io = ImGui::GetIO();
					if (io.MouseDoubleClicked[0])
					{
						// TBD
					}
				}

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
				{
					BaseAssetNode* ptr = assetNode.get();
					ImGui::SetDragDropPayload("ASSET_CELL", &ptr, sizeof(BaseAssetNode*));

					// manually set a padding of 20.0 pixel (TBD: remove this magic number in the future)
					BaseResourceImage* img = mResourceManager->LoadByURL<BaseResourceImage>("EditorAsset\\icon\\material.png");
					ImGui::Image(img->GetImGuiRuntimeID(), ImVec2(iconSize, iconSize));

					ImGui::Text(assetNode->mID.c_str());
					ImGui::EndDragDropSource();
				}

				// manually set a padding of 20.0 pixel (TBD: remove this magic number in the future)
				ImGui::SetCursorPos(ImVec2(pos.x + 20.0f, pos.y + 20.0f));
				BaseResourceImage* img = mResourceManager->LoadByURL<BaseResourceImage>("EditorAsset\\icon\\material.png");
				ImGui::Image(img->GetImGuiRuntimeID(), ImVec2(iconSize - 40.0f, iconSize - 40.0f));

				ImGui::SetCursorPos(ImVec2(pos.x, pos.y + iconSize));
			} else {
				// icon fallback
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

			// change to new line if icon reaches the max size
			posX += iconSize;
			if (posX + iconSize < vMax.x - vMin.x)
			{
				ImGui::SameLine();
			} else {
				posX = 0.0f;
			}

		}
	}

	ifRightClickCurrentWindow(
		[&]() {
			ImGui::OpenPopup("RightClickAssetContent");
		}
	);

	if (ImGui::BeginPopup("RightClickAssetContent"))
	{
		if (ImGui::Button("Load Asset"))
		{
			std::string filePath = ShowFileWindow();
			std::string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
			std::string toPath = mNowSelectedAssetNodeDir->GetURL();

			if (filePath != "<failed>")
			{
				if (toPath.length() > 0)
				{
					toPath += "\\" + fileName;
				}
				else {
					// the base URL is root
					toPath = fileName;
				}

				mAssetManager->RegisterAsset(
					toPath,
					filePath
				);
			}

			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void CoreAssetWidget::DFSConstructExploererNode(BaseAssetNode* nowNode)
{
	bool hasChild;
	int cnt = 0;
	for (std::unique_ptr<BaseAssetNode>& assetNode : nowNode->mSubAssets)
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
			for (std::unique_ptr<BaseAssetNode>& assetNode :nowNode->mSubAssets)
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
	ifLeftClickCurrentWindow(
		[&] () {
			mNowSelectedAssetNodeDir = nullptr;
		}
	);

	DFSConstructExploererNode(mAssetManager->mRootAsset.get());
}

void CoreAssetWidget::Start(ImGuiID& dockspace_id)
{
	ImGuiID dock_down_id = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.25f, nullptr, &dockspace_id);

	ImGui::DockBuilderDockWindow(mDisplayName.c_str(), dock_down_id);
}