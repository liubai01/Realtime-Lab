#pragma once
#include "../../Base/BaseDrawContext.h"

class CoreDrawEdgeContext : public BaseDrawContext
{
public:
	CoreDrawEdgeContext(ID3D12Device* device);

	void InitPSO();
};

