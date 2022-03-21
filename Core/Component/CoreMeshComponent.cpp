#include "CoreMeshComponent.h"

CoreMeshComponent::CoreMeshComponent(shared_ptr<CoreGeometry>& geo)
{
  mGeo = geo;
  mUploaded = false;
}

void CoreMeshComponent::Upload(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
  mUploaded = true;
  UploadGeo<CoreVertex>(*mGeo, device, commandList);
}