#include "BaseResource.h"
#include "../BaseUUIDGenerator.h"

BaseResource::BaseResource(ID3D12Device* device, const string& path)
{
    mName = path.substr(path.find_last_of("/\\") + 1);
    mUUID = uuid::generate_uuid();
    mType = BaseResourceType::RESOURCE_UNKNOWN;
    mUpload = false;
    mDevice = device;
}
