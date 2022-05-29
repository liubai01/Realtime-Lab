#include "BaseScene.h"

BaseScene::BaseScene(BaseGameObjectManager* gOManager)
{
	mGOManager = gOManager;
}

std::shared_ptr<BaseObject> BaseScene::CreateObject(const std::string& name)
{
	std::shared_ptr<BaseObject> ret = mGOManager->CreateObject(name);
	if (ret)
	{
		mRootObjects.push_back(ret);
	}

	return ret;
}