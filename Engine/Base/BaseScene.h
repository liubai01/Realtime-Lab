#pragma once
#include "./BaseGameObjectManager.h"
#include "./BaseCamera.h"
#include "./BaseMainHeap.h"
#include "./BaseRuntimeHeap.h"

class BaseScene
{
public:
	BaseScene(ID3D12Device* device, BaseGameObjectManager* gOManager);
	std::shared_ptr<BaseObject> CreateObject(const std::string& name);

	std::unique_ptr<BaseCamera> mEditorCamera;

	void RegisterMainHandle(BaseMainHeap* mainHeap);
	void RegisterRuntimeHandle(BaseRuntimeHeap* runtimeHeap);

private:
	BaseGameObjectManager* mGOManager;
	std::vector<std::shared_ptr<BaseObject>> mRootObjects;
};

