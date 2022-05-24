#pragma once

#include "../Core/Component/CoreMeshComponent.h"
#include "../Core/CoreMaterialManager.h"
#include "../Base/Resource/BaseResourceMesh.h"
#include <string>


using namespace std;

class CoreMeshLoader
{
public:
	CoreMeshLoader(CoreMaterialManager* matManager);
	
	shared_ptr<CoreMeshComponent> MakeComponent(BaseResourceMesh* resourceMesh);
private:
	CoreMaterialManager* mMatManager;
};

