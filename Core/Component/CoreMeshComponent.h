#pragma once
#include "../../Base/Component/BaseMesh.h"
#include "../../Base/Component/BaseComponent.h"
#include "../CoreGeometry.h"
#include "../CoreMaterial.h"

class CoreMeshComponent : public BaseComponent
{
public:
  vector<shared_ptr<CoreGeometry>> mGeo;
  vector<shared_ptr<CoreMaterial>> mMat;
  vector<unique_ptr<BaseMesh>> mMesh;

  bool mUploaded;

  CoreMeshComponent();

  void AddGeometry(shared_ptr<CoreGeometry> geo, shared_ptr<CoreMaterial> mat=nullptr);

  void Upload(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
  void Render(ID3D12GraphicsCommandList* commandList, int matRegIdx=-1);
};

