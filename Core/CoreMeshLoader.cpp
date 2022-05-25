#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
// Optional. define TINYOBJLOADER_USE_MAPBOX_EARCUT gives robust trinagulation. Requires C++11
//#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#endif // !TINYOBJLOADER_IMPLEMENTATION

#include "CoreMeshLoader.h"
#include "CoreGeometry.h"
#include <unordered_map>
#include "../DebugOut.h"

using namespace std;

CoreMeshLoader::CoreMeshLoader(CoreMaterialManager* matManager)
{
    mMatManager = matManager;
}

shared_ptr<CoreMeshComponent> CoreMeshLoader::MakeComponent(BaseResourceMesh* resourceMesh)
{
    shared_ptr<CoreMeshComponent> ret = make_shared<CoreMeshComponent>();

    for (BaseResourceGeometry& geoSrc: resourceMesh->mGeos)
    {
        shared_ptr<CoreGeometry> geo = make_shared<CoreGeometry>();
        geo->mName = geoSrc.mName;
        geo->mGeoType = CoreGeometryType::GEO_ASSET;
        geo->mID = resourceMesh->mUUID;

        for (size_t idx = 0; idx < geoSrc.mVertices.size(); idx++)
        {
            geo->mIndices.push_back(geoSrc.mIndices[idx]);
            BaseResourceVertex& v = geoSrc.mVertices[idx];
            geo->mVertices.push_back(
                { 
                    v.pos, 
                    v.normal,
                    v.texCoord
                }
            );
        }
        geo->ComputeTangentSpace();
        ret->AddGeometry(geo, mMatManager->CreateMaterial(geo->mName));
    }

    return ret;
}