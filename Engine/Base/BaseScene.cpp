#include "BaseScene.h"

BaseScene::BaseScene(BaseGameObjectManager* gOManager)
{
	mGOManager = gOManager;
}

shared_ptr<BaseObject> BaseScene::CreateObject(const string& name)
{
	shared_ptr<BaseObject> ret = mGOManager->CreateObject(name);
	if (ret)
	{
		mRootObjects.push_back(ret);
	}

	return ret;
}