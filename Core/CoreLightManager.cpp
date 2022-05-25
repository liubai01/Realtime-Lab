#include "CoreLightManager.h"

CoreLightManager::CoreLightManager(ID3D12Device* device) : mLightData(device)
{
	// Default light setup
	mLightData.mBuffer.mData.Ims = { 0.5f, 0.5f, 0.5f, 1.0f };
	mLightData.mBuffer.mData.Ia = { 0.2f, 0.2f, 0.2f, 1.0f };
	mLightData.mBuffer.mData.Id = { 1.0f, 1.0f, 1.0f, 1.0f };
	mLightData.mBuffer.mData.LightDir = { 0.577f, 0.577f, 0.577f, 0.0f };
}

void CoreLightManager::RegisterMainHandle(BaseMainHeap* mainHeap)
{
	mLightData.RegisterMainHandle(mainHeap);
}

void CoreLightManager::RegisterRuntimeHandle(BaseRuntimeHeap* runtimeHeap)
{
	mLightData.RegisterRuntimeHandle(runtimeHeap);
}


// Check whether there is dead component
// and migrate data into manager
void CoreLightManager::Update()
{
	// clean dead component
	auto nowIter = mLightComponents.begin();

	while (nowIter != mLightComponents.end())
	{
		if (nowIter->use_count() == 1)
		{
			nowIter = mLightComponents.erase(nowIter);
		} else {
			++nowIter;
		}
	}

	if (!mLightComponents.empty())
	{
		mLightData.mBuffer.mData = mLightComponents.front()->mData;
	}
	else {
		// unlit everything
		mLightData.mBuffer.mData.Ims = { 0.0f, 0.0f, 0.0f, 1.0f };
		mLightData.mBuffer.mData.Ia = { 0.0f, 0.0f, 0.0f, 1.0f };
		mLightData.mBuffer.mData.Id = { 0.0f, 0.0f, 0.0f, 1.0f };
		mLightData.mBuffer.mData.LightDir = { 0.577f, 0.577f, 0.577f, 0.0f };
	}
}

shared_ptr<CoreLightComponent> CoreLightManager::MakeLightComponent()
{
	shared_ptr<CoreLightComponent> ret = make_shared<CoreLightComponent>();
	mLightComponents.push_back(ret);
	return ret;
}