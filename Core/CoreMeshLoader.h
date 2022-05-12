#pragma once

#include "../Core/Component/CoreMeshComponent.h"
#include "../Core/CoreMaterialManager.h"
#include "../ThirdParty/tiny_obj_loader.h"
#include <string>


using namespace std;

class CoreMeshLoader
{
public:
	CoreMeshLoader(CoreMaterialManager* matManager);
	
	shared_ptr<CoreMeshComponent> LoadObjMesh(string path, string name);
private:
	CoreMaterialManager* mMatManager;
};

