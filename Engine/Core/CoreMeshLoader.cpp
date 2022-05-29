#include "CoreMeshLoader.h"
#include "CoreGeometry.h"
#include <unordered_map>
#include "../DebugOut.h"

using namespace std;

CoreMeshLoader::CoreMeshLoader(BaseAssetManager* assetManager)
{
    mAssetManager = assetManager;
}

shared_ptr<CoreMeshComponent> CoreMeshLoader::MakeComponent(BaseResourceMesh* resourceMesh)
{
    shared_ptr<CoreMeshComponent> ret = make_shared<CoreMeshComponent>(mAssetManager);
    BindResource2MeshComponent(ret, resourceMesh);

    return ret;
}

void CoreMeshLoader::BindResource2MeshComponent(shared_ptr<CoreMeshComponent> ret, BaseResourceMesh* resourceMesh)
{
    ret->ClearGeometry();
    ret->mMeshType = CoreMeshComponentType::ASSET_COMPONENT;
    ret->mID = resourceMesh->mUUIDAsset;

    for (BaseResourceGeometry& geoSrc : resourceMesh->mGeos)
    {
        // TBD: a cache system of geometry
        shared_ptr<CoreGeometry> geo = make_shared<CoreGeometry>();
        geo->mName = geoSrc.mName;

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
        ret->AddGeometry(geo);
    }
}