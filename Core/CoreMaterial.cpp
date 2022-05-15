#include "CoreMaterial.h"

CoreMaterial::CoreMaterial(ID3D12Device* device): BaseStagedBuffer(device) {
	mBuffer.mData.Kd = { 0.8f, 0.8f, 0.8f, 1.0f }; // Diffuse Color Constant
	mBuffer.mData.Ks = { 0.5f, 0.5f, 0.5f, 1.0f }; // Specular Color Constant
	mBuffer.mData.Ns = 323.999994f; // Shiniess
	mBuffer.mData.NormalStrength = 1.0f;
	
	mBuffer.mData.isBaseColorTextured = false;
	mBuffer.mData.isNormalTextured = false;

	mName = "Unnamed";
}

void CoreMaterial::SetDiffuseColorTextured(shared_ptr<BaseImage> diffuseColorTexture)
{
	mBuffer.mData.isBaseColorTextured = true;
	mDiffuseColorTexture = diffuseColorTexture;
}

void CoreMaterial::SetNormalTextured(shared_ptr<BaseImage> normalMapTexture)
{
	mBuffer.mData.isNormalTextured = true;
	mNormalMapTexture = normalMapTexture;
}

void CoreMaterial::SetNormalStrength(float val)
{
	mBuffer.mData.NormalStrength = val;
}

void CoreMaterial::Upload()
{

}