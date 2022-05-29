#include "BaseObject.h"
#include "../MathUtils.h"

void BaseObject::AddComponent(std::shared_ptr<BaseComponent> component)
{
  mComponents.push_back(component);
  mChildObjects.clear();
  mParentObject = nullptr;
}

std::string BaseObject::GetUUID()
{
	return mUuid;
}


BaseObject* BaseObject::GetParent()
{
	return mParentObject;
}

void BaseObject::SetParent(BaseObject* obj)
{
	// remove its pointer at its parent
	if (mParentObject)
	{
		if (mParentObject->mChildObjects.find(obj) != mParentObject->mChildObjects.end())
		{
			mParentObject->mChildObjects.erase(obj);
		}
		
	} else {
		//dout::printf("erase root obj: %s\n", obj->mName.c_str());
		mRootObjects->erase(this);
	}

	if (obj)
	{
		mParentObject = obj;
		// update the transform's parent to implement the hierachy
		mTransform.mParent = &obj->mTransform;
		mParentObject->mChildObjects.insert(this);
	} else {
		mParentObject = nullptr;
		mRootObjects->insert(this);
	}
	
}

void BaseObject::DispatchTransformUpload(BaseRuntimeHeap* runtimeHeap)
{
	mTransform.RegisterRuntimeHandle(runtimeHeap);

	for (auto& obj : mChildObjects)
	{
		obj->mTransform.RegisterRuntimeHandle(runtimeHeap);
	}
}


const std::unordered_set<BaseObject*>& BaseObject::GetChildObjects()
{
	return mChildObjects;
}