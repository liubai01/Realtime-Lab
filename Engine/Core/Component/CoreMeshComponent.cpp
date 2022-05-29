#include "CoreMeshComponent.h"
#include "../../ThirdParty/ImGUI/imgui.h"
#include <type_traits>

template <typename E>
constexpr auto to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}

CoreMeshComponent::CoreMeshComponent(BaseAssetManager* assetManager)
{
    mAssetManager = assetManager;
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
void CoreMeshComponent::AddGeometry(shared_ptr<CoreGeometry> geo, CoreResourceMaterial* mat)
{

    // the geometry info (could be potentially shared by multiple mesh component)
    mGeo.push_back(geo);
    // the material
    mMat.push_back(mat);
    // the runtime mesh data (should be uploaded to GPU before accessing vertexBuffer & indexBuffer)
    mMesh.push_back(make_unique<BaseMesh>());
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


void CoreMeshComponent::OnEditorGUI()
{
    if (ImGui::TreeNodeEx("Mesh Component", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::Text("Mesh ");
        ImGui::SameLine();

        if (mMeshType == CoreMeshComponentType::ASSET_COMPONENT)
        {
            BaseAssetNode* node = mAssetManager->LoadAssetByUUID(mID);

            if (node)
            {
                //ImGui::Text(node->mID.c_str());
                char name[128]{ 0 };
                strcpy_s(name, node->mID.c_str());

                ImGui::InputText("NameMesh", name, IM_ARRAYSIZE(name), ImGuiInputTextFlags_ReadOnly);

                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_CELL"))
                    {
                        BaseAssetNode* node = nullptr;
                        memcpy(&node, payload->Data, sizeof(BaseAssetNode*));
                    }

                    ImGui::EndDragDropTarget();
                }
            }
            else {
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
    vector<json> geoMeta = vector<json>();
    
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