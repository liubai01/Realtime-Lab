#include "BaseGameObjectManager.h"
#include "BaseUUIDGenerator.h"

BaseGameObjectManager::BaseGameObjectManager(ID3D12Device* device, BaseMainHeap* mainHeap)
{
    mDevice = device;
    mMainHeap = mainHeap;
}

shared_ptr<BaseObject> BaseGameObjectManager::CreateObject(const string& name)
{
    string uuid = uuid::generate_uuid();
    auto f = mObjs.find(uuid);
    if (f != mObjs.end())
    {
        dout::printf("[BaseGameObjectManager] Create Object UUID %s Repeated!", name.c_str());
        return nullptr;
    }
    shared_ptr<BaseObject> obj = make_shared<BaseObject>(name, uuid, mDevice, &mRootObjects);
    obj->mTransform.RegisterMainHandle(mMainHeap);
    mRootObjects.insert(obj.get());

    mObjs[uuid] = obj;

    return obj;
}

shared_ptr<BaseObject>BaseGameObjectManager::GetObject(const string& uuid)
{
    auto f = mObjs.find(uuid);

    if (f != mObjs.end())
    {
        return f->second;
    }

    dout::printf("[BaseGameObjectManager] Object %s not found!", uuid.c_str());

    return nullptr;
}

void BaseGameObjectManager::DispatchTransformUpload(BaseRuntimeHeap* runtimeHeap)
{
    for (auto& obj : mRootObjects)
    {
        //dout::printf("root obj: %s\n", obj->mName.c_str());
        obj->DispatchTransformUpload(runtimeHeap);
    }
}