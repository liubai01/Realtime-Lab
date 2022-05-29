#pragma once
#include "./BaseGameObjectManager.h"

class BaseScene
{
public:
	BaseScene(BaseGameObjectManager* gOManager);
	std::shared_ptr<BaseObject> CreateObject(const std::string& name);

private:
	BaseGameObjectManager* mGOManager;
	std::vector<std::shared_ptr<BaseObject>> mRootObjects;
};

