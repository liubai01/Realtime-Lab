#include "BaseResource.h"
#include "../BaseUUIDGenerator.h"
#include "../../DebugOut.h"

BaseResource::BaseResource(ID3D12Device* device, BaseAssetNode* assetNode)
{
    mUUIDResource = uuid::generate_uuid();
    mType = BaseResourceType::RESOURCE_UNKNOWN;
    mUpload = false;
    mDevice = device;
    mIsRuntimeResource = false;

    if (assetNode)
    {
        mName = assetNode->mFullPath.substr(assetNode->mFullPath.find_last_of("/\\") + 1);
        mUUIDAsset = assetNode->mUUID;
    }
    else {
        mName = "unknown";
        dout::printf("[BaseResource] The assetNode is nullptr when intializing a resource.");
    }

}
