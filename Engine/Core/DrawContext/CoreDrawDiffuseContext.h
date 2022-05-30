#pragma once
#include "../../Base/BaseDrawContext.h"

class CoreDrawDiffuseContext: public BaseDrawContext
{
public:
	CoreDrawDiffuseContext(ID3D12Device* device, BaseAssetManager* assetManager);

	void InitPSO();
};

