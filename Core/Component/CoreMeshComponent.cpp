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
    mUploaded = false;
}

void CoreMeshComponent::AddGeometry(shared_ptr<CoreGeometry> geo, shared_ptr<CoreMaterial> mat)
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
            commandList->SetGraphicsRootDescriptorTable(matRegIdx, mat->GetRuntimeHandle().GetGPUHandle());
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

        ImGui::Separator();
        ImGui::TreePop();
    }
}

json CoreMeshComponent::Serialize()
{
    vector<json> geoMeta;
    for (shared_ptr<CoreGeometry>& geo : mGeo)
    {
        geoMeta.push_back(json{
            { "geoType", to_underlying(geo->mGeoType)},
            { "geoID", geo->mID }
        });
    }

    json j = json{
        {"GeometryMetadata", geoMeta}
    };
    return j;
}

void CoreMeshComponent::Deserialize(const json& j)
{

}