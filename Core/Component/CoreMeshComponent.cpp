#include "CoreMeshComponent.h"
#include "../../ThirdParty/ImGUI/imgui.h"

CoreMeshComponent::CoreMeshComponent()
{
	mComponentType = ComponentType::COMPONENT_MESH;
    mUploaded = false;
}

void CoreMeshComponent::AddGeometry(shared_ptr<CoreGeometry> geo, shared_ptr<CoreMaterial> mat)
{
    mGeo.push_back(geo);
    mMat.push_back(mat);
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