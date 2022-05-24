#pragma once
#include "../Base/BaseStagedBuffer.h"

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "../DebugOut.h"
#include "../MathUtils.h"
//#include "../Base/BaseImage.h"
#include "../Base/Resource/BaseResourceImage.h"
#include <time.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

struct CoreMaterialConsts {
  XMFLOAT4 Kd = { 0.8f, 0.8f, 0.8f, 1.0f }; // Diffuse Color Constant
  XMFLOAT4 Ks = { 0.5f, 0.5f, 0.5f, 1.0f }; // Specular Color Constant
  float Ns = 323.999994f; // Shiniess
  int isBaseColorTextured = false;
  int isNormalTextured = false;
  float NormalStrength = 1.0f; 
};

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

