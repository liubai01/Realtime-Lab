#pragma once
#include "../Base/BaseStagedBuffer.h"

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "../DebugOut.h"
#include "../MathUtils.h"
#include "CoreStructs.h"
#include "Component/CoreLightComponent.h"
#include <list>

class CoreLightManager
{
public:
	CoreLightManager(ID3D12Device* device);

	std::shared_ptr<CoreLightComponent> MakeLightComponent();

	// Check whether there is dead component
	// and migrate data into manager
	void Update();

	void RegisterMainHandle(BaseMainHeap* mainHeap);
	void RegisterRuntimeHandle(BaseRuntimeHeap* runtimeHeap);

	std::list<std::shared_ptr<CoreLightComponent>> mLightComponents;
	BaseStagedBuffer<CoreDirectLightConsts> mLightData;
};

