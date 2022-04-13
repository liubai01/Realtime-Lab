#pragma once
#include "../Base/BaseStagedBuffer.h"

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "../DebugOut.h"
#include "../MathUtils.h"

struct CoreDirectLightConsts {
	XMFLOAT4 Ims = { 1.0f, 1.0f, 1.0f, 1.0f }; // intensity of specular
	XMFLOAT4 Ia = { 0.3f, 0.3f, 0.3f, 1.0f }; // intensity of ambient
	XMFLOAT4 Id = { 1.0f, 1.0f, 1.0f ,1.0f }; // intensity of diffuse
	XMFLOAT4 LightDir = { 0.577f, 0.577f, 0.577f, 0.0f}; // light direction
};


class CoreLightManager
{
public:
	CoreLightManager(ID3D12Device* device);

	void SetLightDir(float x, float y, float z);

	void RegisterMainHandle(BaseMainHeap* mainHeap);
	void RegisterRuntimeHandle(BaseRuntimeHeap* runtimeHeap);

	BaseStagedBuffer<CoreDirectLightConsts> mLightData;
};

