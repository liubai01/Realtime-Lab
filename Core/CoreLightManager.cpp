#include "CoreLightManager.h"

void CoreLightManager::SetLightDir(float x, float y, float z)
{
	mLightData.mBuffer.mData.LightDir = { x, y, z, 0.0f };
}

CoreLightManager::CoreLightManager(ID3D12Device* device) : mLightData(device)
{
	// Default light setup
	mLightData.mBuffer.mData.Ims = { 1.0f, 1.0f, 1.0f, 1.0f };
	mLightData.mBuffer.mData.Ia = { 0.1f, 0.1f, 0.1f, 1.0f };
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