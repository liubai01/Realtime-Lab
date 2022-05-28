#pragma once
#include "../Base/BaseStagedBuffer.h"

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "../DebugOut.h"
#include "../MathUtils.h"
#include "../Base/Resource/BaseResourceImage.h"
#include "./CoreStructs.h"
#include <time.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

class CoreMaterial: public BaseStagedBuffer<CoreMaterialConsts>
{
public:
	string mName;
	BaseResourceImage* mDiffuseColorTexture;
	BaseResourceImage* mNormalMapTexture;

	CoreMaterial(ID3D12Device* device);

	void SetDiffuseColorTextured(BaseResourceImage* diffuseColorTexture);
	void SetNormalTextured(BaseResourceImage* normalMapTexture);
	void SetNormalStrength(float val);

	void Upload();
};

