#include "CoreMeshComponent.h"
#include "../../ThirdParty/ImGUI/imgui.h"
#include <type_traits>

template <typename E>
constexpr auto to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}

CoreMeshComponent::CoreMeshComponent()
{
	mComponentType = ComponentType::COMPONENT_MESH;
    mMeshType = CoreMeshComponentType::UNKNOWN_COMPONENT;
    mID = "<unknown>";

    mUploaded = false;
}

void CoreMeshComponent::ClearGeometry()
{
    mGeo.clear();
    mMat.clear();
    mMesh.clear();

    mMeshType = CoreMeshComponentType::UNKNOWN_COMPONENT;
}


/*
*  CoreMeshComponent::AddGeometry is invoked from CoreMeshLoader.
* 
*  Before adding geometry, you should set mMeshType and mID which notifies its source.
*  If the source is not set correctly, (de)serialization would fail.
*/
void CoreMeshComponent::AddGeometry(std::shared_ptr<CoreGeometry> geo, CoreResourceMaterial* mat)
{

    // the geometry info (could be potentially shared by multiple mesh component)
    mGeo.push_back(geo);
    // the material
    mMat.push_back(mat);
    // the runtime mesh data (should be uploaded to GPU before accessing vertexBuffer & indexBuffer)
    mMesh.push_back(std::make_unique<BaseMesh>());
}

void CoreMeshComponent::Upload(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
    // upload the geometry from CPU to GPU
    mUploaded = true;
    for (int i = 0; i < mMesh.size(); ++i)
    {
        auto& mesh = mMesh[i];
        auto& geo = mGeo[i];
        mesh->UploadGeo<CoreVertex>(*geo, device, commandList);
    }
}

void CoreMeshComponent::Render(ID3D12GraphicsCommandList* commandList, int matRegIdx)
{
    for (int i = 0; i < mMesh.size(); ++i)
    {
        auto& mesh = mMesh[i];
        auto& mat = mMat[i];
        auto& geo = mGeo[i];

        if (matRegIdx > 0)
        {
            commandList->SetGraphicsRootDescriptorTable(matRegIdx, mat->mRuntimeHandle.GetGPUHandle());
        }

        D3D12_INDEX_BUFFER_VIEW ibView = mesh->mIndexBufferView;
        D3D12_VERTEX_BUFFER_VIEW vbView = mesh->mVertexBufferView;

        commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->IASetVertexBuffers(0, 1, &vbView);
        commandList->IASetIndexBuffer(&ibView);

        commandList->DrawIndexedInstanced(
            static_cast<UINT>(geo->mIndices.size()),
            1, 0, 0, 0
        );
    }

}


void CoreMeshComponent::OnEditorGUI(BaseAssetManager* assetManager, BaseResourceManager* resourceManager)
{
    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::TreeNodeEx("Mesh Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
    {

        ImGui::Text("Mesh ");
        ImGui::SameLine();

        // TBD: support primitive
        if (mMeshType == CoreMeshComponentType::ASSET_COMPONENT)
        {
            BaseAssetNode* node = assetManager->LoadAssetByUUID(mID);

            if (node)
            {
                // Show the slot of the mesh
                char name[128]{ 0 };
                strcpy_s(name, node->mID.c_str());
                // the slot is rendered as a read-only input-text
                // TBD: support search for asset of resource mesh in the future
                ImGui::InputText("NameMesh", name, IM_ARRAYSIZE(name), ImGuiInputTextFlags_ReadOnly);

                // Enable drag & drop of the slot of mesh
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_CELL"))
                    {
                        BaseAssetNode* node = nullptr;
                        memcpy(&node, payload->Data, sizeof(BaseAssetNode*));

                        // TBD: change the mesh
                    }

                    ImGui::EndDragDropTarget();
                }

                // Show the material slots
                int flag = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;
                bool treeNodeOpen = ImGui::TreeNodeEx("Matereial", flag, "Material");

                if (treeNodeOpen)
                {
                    for (int i = 0; i < mMat.size(); ++i)
                    {
                        ImGui::BulletText("Mat[%d]: ", i);
                        ImGui::SameLine();
                        
                        // check whether material exists
                        if (!mMat[i])
                        {
                            char namemat[128]{ 0 };
                            strcpy_s(namemat, "<Empty>");
                            ImGui::InputText("NameMat", namemat, IM_ARRAYSIZE(namemat), ImGuiInputTextFlags_ReadOnly);
                        }
                        else {
                            char namemat[128]{ 0 };
                            strcpy_s(namemat, mMat[i]->mName.c_str());
                            ImGui::InputText("NameMat", namemat, IM_ARRAYSIZE(namemat), ImGuiInputTextFlags_ReadOnly);
                        }

                        // Enable drag & drop of the matereial slot
                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_CELL"))
                            {
                                BaseAssetNode* node = nullptr;
                                memcpy(&node, payload->Data, sizeof(BaseAssetNode*));
                                
                                if (node->GetAssetType() == BaseAssetType::ASSET_MATERIAL)
                                {
                                    mMat[i] = resourceManager->LoadByURL<CoreResourceMaterial>(node->GetURL());
                                }
                                
                            }

                            ImGui::EndDragDropTarget();
                        }
                    }
                    ImGui::TreePop();
                }
            }
            else {
                // `node == nullptr` branch
                // when the reference is broken, clean the component
                ClearGeometry();
                ImGui::Text("<Broken Asset>");
            }

        }

        ImGui::Separator();
        ImGui::TreePop();
    }
}

json CoreMeshComponent::Serialize()
{
    std::vector<json> geoMeta = std::vector<json>();
    
    //for (int i = 0; i < mGeo.size(); ++i)
    //{
    //    geoMeta.push_back(json{
    //        { "GeometryType", to_underlying(mGeo[i]->mGeoType)},
    //        { "GeometryID", mGeo[i]->mID },
    //        { "Matereial", mMat[i] ? mMat[i]->mUUIDAsset : "<null>"}
    //    });
    //}

    json j = json{
        {"GeometryMetadata", geoMeta}
    };
    return j;
}

void CoreMeshComponent::Deserialize(const json& j)
{

}