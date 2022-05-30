#pragma once
#include "../../Base/BaseDrawContext.h"

class CoreDrawBlurContext : public BaseDrawContext
{
public:
	CoreDrawBlurContext(ID3D12Device* device, BaseAssetManager* assetManager);

	void InitPSO();
};

