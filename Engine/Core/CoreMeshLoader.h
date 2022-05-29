#pragma once

#include "../Core/Component/CoreMeshComponent.h"
#include "../Base/Resource/BaseResourceMesh.h"
#include <string>
#include "../Base/Asset/BaseAssetManager.h"


using namespace std;

class CoreMeshLoader
{
public:
	CoreMeshLoader(BaseAssetManager* assetManager);

	shared_ptr<CoreMeshComponent> MakeComponent(BaseResourceMesh* resourceMesh);
	void BindResource2MeshComponent(shared_ptr<CoreMeshComponent>, BaseResourceMesh* resourceMesh);
private:
	BaseAssetManager* mAssetManager;
};
