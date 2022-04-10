#pragma once
#include "../../Base/Component/BaseMeshComponent.h"
#include "../CoreGeometry.h"
#include "../CoreMaterial.h"

class CoreMeshComponent : public BaseMeshComponent
{
public:
  shared_ptr<CoreGeometry> mGeo;
  shared_ptr<CoreMaterial> mMat;

  bool mUploaded;

  CoreMeshComponent(shared_ptr<CoreGeometry>& geo);

  void Upload(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
};

