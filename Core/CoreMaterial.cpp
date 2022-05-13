#include "CoreMaterial.h"

CoreMaterial::CoreMaterial(ID3D12Device* device): BaseStagedBuffer(device) {
	mBuffer.mData.Kd = { 0.8f, 0.8f, 0.8f, 1.0f }; // Diffuse Color Constant
	mBuffer.mData.Ks = { 0.5f, 0.5f, 0.5f, 1.0f }; // Specular Color Constant
	mBuffer.mData.Ns = 323.999994f; // Shiniess

	mName = "Unnamed";
}

void CoreMaterial::SetDiffuseColorTextured(shared_ptr<BaseImage> diffuseColorTexture)
{
	mBuffer.mData.isDiffuseColorTextured = true;
	mDiffuseColorTexture = diffuseColorTexture;
}

void CoreMaterial::Upload()
{

}