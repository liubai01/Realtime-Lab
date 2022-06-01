#include "CoreMaterialWidget.h"

CoreMaterialWidget::CoreMaterialWidget(BaseResourceManager* resourceManager, BaseAssetManager* assetManager)
{
	mIsShow = false;
	mSetFocusThisFrame = false;

	mDisplayName = "Matereial Editor";
	mMatResourceUUID = "<empty>";

	mResourceManager = resourceManager;
	mAssetManager = assetManager;
}

void CoreMaterialWidget::AssignMat(CoreResourceMaterial* mat)
{
	mMatResourceUUID = mat->mUUIDResource;

	mIsBaseColorTextured = mat->mStagedBuffer->mBuffer.mData.isBaseColorTextured;
	mIsNormalMapTextured = mat->mStagedBuffer->mBuffer.mData.isNormalTextured;
	mIsMetallicTextured = mat->mStagedBuffer->mBuffer.mData.isMetallicTextured;
	mIsRoughnessTextured = mat->mStagedBuffer->mBuffer.mData.isRoughnessTextured;
}

void DragAndDropImage(std::string& uuid)
{
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_CELL"))
		{
			BaseAssetNode* node = nullptr;
			memcpy(&node, payload->Data, sizeof(BaseAssetNode*));

			if (node->GetAssetType() == BaseAssetType::ASSET_IMAGE)
			{
				uuid = node->mUUID;
			}

		}

		ImGui::EndDragDropTarget();
	}
}

void CoreMaterialWidget::Update()
{
	ImVec2 ImgSize = ImVec2(120.0f, 120.0f);
	ImGui::Begin(mDisplayName.c_str(), &mIsShow);

	if (mSetFocusThisFrame)
	{
		ImGui::SetWindowFocus();
	}

	mSetFocusThisFrame = false;

	BaseResourceImage* img = mResourceManager->LoadByURL<BaseResourceImage>("EditorAsset\\icon\\material.png");
	ImGui::Image(img->GetImGuiRuntimeID(), ImVec2(32.0f, 32.0f));
	ImGui::SameLine();
	ImGui::Text("Material ");
	ImGui::NewLine();

	if (mMatResourceUUID != "<empty>")
	{
		CoreResourceMaterial* mat = mResourceManager->LoadByUUID<CoreResourceMaterial>(mMatResourceUUID);
		if (mat)
		{
			// Show the slot of the mesh
			char name[128]{ 0 };
			strcpy_s(name, mat->mName.c_str());
			ImGui::InputText("NameMat", name, IM_ARRAYSIZE(name), ImGuiInputTextFlags_ReadOnly);

			// --- base color texture ---
			ImGui::Separator();
			if (mIsBaseColorTextured) {
				ImGui::Checkbox("Base Color Texture: ", &mIsBaseColorTextured);
				if (mat->mBaseColorTextureUUID != "<empty>")
				{
					// only set the flag when texture is avaliable
					mat->mStagedBuffer->mBuffer.mData.isBaseColorTextured = true;
					BaseAssetNode* node = mAssetManager->LoadAssetByUUID(mat->mBaseColorTextureUUID);
					BaseResourceImage* img = mResourceManager->LoadByURL<BaseResourceImage>(node->GetURL());

					if (img->GetImGuiRuntimeID())
					{
						ImGui::Image(img->GetImGuiRuntimeID(), ImgSize);
						DragAndDropImage(mat->mBaseColorTextureUUID);
					}

					ImGui::Text(node->mID.c_str());

				}
				else {
					strcpy_s(name, "<empty>");

					BaseResourceImage* img = mResourceManager->LoadByURL<BaseResourceImage>("EditorAsset\\icon\\image.png");
					ImGui::Image(
						img->GetImGuiRuntimeID(), ImgSize, 
						ImVec2(0, 0), ImVec2(1, 1), 
						ImVec4(0.4f, 0.4f, 0.4f, 1.0f), // dark
						ImVec4(1.0f, 1.0f ,1.0f, 0.5f) // border
					);

					DragAndDropImage(mat->mBaseColorTextureUUID);
				}
			}
			else {
				mat->mStagedBuffer->mBuffer.mData.isBaseColorTextured = false;
				ImGui::Checkbox("Base Color Texture", &mIsBaseColorTextured);
				
				ImGui::ColorEdit3("BaseColor##", (float*)&mat->mStagedBuffer->mBuffer.mData.baseColor, ImGuiColorEditFlags_NoLabel);
			}
			ImGui::NewLine();

			// --- normal map ---
			ImGui::Separator();
			if (mIsNormalMapTextured) {
				ImGui::Checkbox("Normal Mapping: ", &mIsNormalMapTextured);
				if (mat->mNormalMapTextureUUID != "<empty>")
				{
					ImGui::Text("Normal Strength");
					ImGui::DragFloat("NormalStrength", &mat->mStagedBuffer->mBuffer.mData.normalStrength, 0.01f, 0.00f, 3.00f, "%0.2f");

					// only set the flag when texture is avaliable
					mat->mStagedBuffer->mBuffer.mData.isNormalTextured = true;
					BaseAssetNode* node = mAssetManager->LoadAssetByUUID(mat->mNormalMapTextureUUID);
					BaseResourceImage* img = mResourceManager->LoadByURL<BaseResourceImage>(node->GetURL());

					if (img->GetImGuiRuntimeID())
					{
						ImGui::Image(img->GetImGuiRuntimeID(), ImgSize);
						DragAndDropImage(mat->mNormalMapTextureUUID);
					}
					ImGui::Text(node->mID.c_str());
				}
				else {
					strcpy_s(name, "<empty>");

					BaseResourceImage* img = mResourceManager->LoadByURL<BaseResourceImage>("EditorAsset\\icon\\image.png");
					ImGui::Image(
						img->GetImGuiRuntimeID(), ImgSize,
						ImVec2(0, 0), ImVec2(1, 1),
						ImVec4(0.4f, 0.4f, 0.4f, 1.0f), // dark
						ImVec4(1.0f, 1.0f, 1.0f, 0.5f) // border
					);

					DragAndDropImage(mat->mNormalMapTextureUUID);
				}
			}
			else {
				mat->mStagedBuffer->mBuffer.mData.isNormalTextured = false;
				ImGui::Checkbox("Normal Mapping", &mIsNormalMapTextured);
			}
			ImGui::NewLine();

			// --- roughness ---
			ImGui::Separator();
			if (mIsRoughnessTextured) {
				ImGui::Checkbox("Roughness Texture: ", &mIsRoughnessTextured);
				if (mat->mRoughnessTextureUUID != "<empty>")
				{
					// only set the flag when texture is avaliable
					mat->mStagedBuffer->mBuffer.mData.isRoughnessTextured = true;
					BaseAssetNode* node = mAssetManager->LoadAssetByUUID(mat->mRoughnessTextureUUID);
					BaseResourceImage* img = mResourceManager->LoadByURL<BaseResourceImage>(node->GetURL());

					if (img->GetImGuiRuntimeID())
					{
						ImGui::Image(img->GetImGuiRuntimeID(), ImgSize);
						DragAndDropImage(mat->mRoughnessTextureUUID);
					}
					ImGui::Text(node->mID.c_str());
				}
				else {
					strcpy_s(name, "<empty>");

					BaseResourceImage* img = mResourceManager->LoadByURL<BaseResourceImage>("EditorAsset\\icon\\image.png");
					ImGui::Image(
						img->GetImGuiRuntimeID(), ImgSize,
						ImVec2(0, 0), ImVec2(1, 1),
						ImVec4(0.4f, 0.4f, 0.4f, 1.0f), // dark
						ImVec4(1.0f, 1.0f, 1.0f, 0.5f) // border
					);

					DragAndDropImage(mat->mRoughnessTextureUUID);
				}
			}
			else {
				mat->mStagedBuffer->mBuffer.mData.isRoughnessTextured = false;
				ImGui::Checkbox("Roughness Texture", &mIsRoughnessTextured);
				ImGui::SliderFloat("Roughness", &mat->mStagedBuffer->mBuffer.mData.roughness, 0.00f, 1.00f, "%0.2f");
			}
			ImGui::NewLine();

			// --- metallic ---
			ImGui::Separator();
			if (mIsMetallicTextured) {
				ImGui::Checkbox("Metallic Texture: ", &mIsMetallicTextured);
				if (mat->mMetallicTextureUUID!= "<empty>")
				{
					// only set the flag when texture is avaliable
					mat->mStagedBuffer->mBuffer.mData.isMetallicTextured = true;
					BaseAssetNode* node = mAssetManager->LoadAssetByUUID(mat->mMetallicTextureUUID);
					BaseResourceImage* img = mResourceManager->LoadByURL<BaseResourceImage>(node->GetURL());

					if (img->GetImGuiRuntimeID())
					{
						ImGui::Image(img->GetImGuiRuntimeID(), ImgSize);
						DragAndDropImage(mat->mMetallicTextureUUID);
					}
					ImGui::Text(node->mID.c_str());
				}
				else {
					strcpy_s(name, "<empty>");

					BaseResourceImage* img = mResourceManager->LoadByURL<BaseResourceImage>("EditorAsset\\icon\\image.png");
					ImGui::Image(
						img->GetImGuiRuntimeID(), ImgSize,
						ImVec2(0, 0), ImVec2(1, 1),
						ImVec4(0.4f, 0.4f, 0.4f, 1.0f), // dark
						ImVec4(1.0f, 1.0f, 1.0f, 0.5f) // border
					);

					DragAndDropImage(mat->mMetallicTextureUUID);
				}
			}
			else {
				mat->mStagedBuffer->mBuffer.mData.isMetallicTextured = false;
				ImGui::Checkbox("Metallic Texture", &mIsMetallicTextured);
				ImGui::SliderFloat("Metallic", &mat->mStagedBuffer->mBuffer.mData.metallic, 0.00f, 1.00f, "%0.2f");
			}
			ImGui::NewLine();

		}
		
	}

	ImGui::End();
}

void CoreMaterialWidget::Start(ImGuiID& dockspace_id)
{
	ImGui::DockBuilderDockWindow(mDisplayName.c_str(), dockspace_id);
}
