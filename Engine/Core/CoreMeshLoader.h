#pragma once

#include "../Core/Component/CoreMeshComponent.h"
#include "../Base/Resource/BaseResourceMesh.h"
#include <string>
#include "../Base/Asset/BaseAssetManager.h"


class CoreMeshLoader
{
public:
	CoreMeshLoader();

	std::shared_ptr<CoreMeshComponent> MakeComponent(BaseResourceMesh* resourceMesh);
	void BindResource2MeshComponent(std::shared_ptr<CoreMeshComponent>, BaseResourceMesh* resourceMesh);
private:
	BaseAssetManager* mAssetManager;
};
