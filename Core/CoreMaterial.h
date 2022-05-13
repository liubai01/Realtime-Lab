#pragma once
#include "../Base/BaseStagedBuffer.h"

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "../DebugOut.h"
#include "../MathUtils.h"
#include "../Base/BaseImage.h"
#include <time.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

struct CoreMaterialConsts {
  XMFLOAT4 Kd = { 0.8f, 0.8f, 0.8f, 1.0f }; // Diffuse Color Constant
  XMFLOAT4 Ks = { 0.5f, 0.5f, 0.5f, 1.0f }; // Specular Color Constant
  float Ns = 323.999994f; // Shiniess
  bool isDiffuseColorTextured = false;
};

class CoreMaterial: public BaseStagedBuffer<CoreMaterialConsts>
{
public:
	string mName;
	shared_ptr<BaseImage> mDiffuseColorTexture;

	CoreMaterial(ID3D12Device* device);

	void SetDiffuseColorTextured(shared_ptr<BaseImage> mDiffuseColorTexture);

	void Upload();
};

