#pragma once
#include "./BaseGameObjectManager.h"

class BaseScene
{
public:
	BaseScene(BaseGameObjectManager* gOManager);
	shared_ptr<BaseObject> CreateObject(const string& name);

private:
	BaseGameObjectManager* mGOManager;
	vector<shared_ptr<BaseObject>> mRootObjects;
};

