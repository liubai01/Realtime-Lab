#include "BaseScene.h"

BaseScene::BaseScene(ID3D12Device* device, BaseGameObjectManager* gOManager)
{
	mGOManager = gOManager;
	// Remark: The scene would resize according to the window size in each frame,
	// Set an arbitary size here
	mEditorCamera = std::make_unique<BaseCamera>(device, 800, 600);
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

void BaseScene::RegisterMainHandle(BaseMainHeap* mainHeap)
{
	mEditorCamera->RegisterMainHandle(mainHeap);
}

void BaseScene::RegisterRuntimeHandle(BaseRuntimeHeap* runtimeHeap)
{
	mEditorCamera->RegisterRuntimeHandle(runtimeHeap);
}
