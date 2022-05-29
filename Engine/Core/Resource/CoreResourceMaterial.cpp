#include "CoreResourceMaterial.h"

CoreResourceMaterial::CoreResourceMaterial(ID3D12Device* device, BaseAssetNode* assetNode) : BaseResource(device, assetNode)
{
	// for runtime deciding the acutal type of BaseResource base pointer
	mType = CoreResourceMaterial::ClassResourceType;

	mStagedBuffer = std::make_unique<BaseStagedBuffer<CoreMaterialConsts>>(device);

	// load meta
	std::ifstream i(assetNode->mFullPath);
	json j;
	i >> j;

	mIsRuntimeResource = true;
	
	Deserialize(j);
}

json CoreResourceMaterial::Serialize()
{
	json j = json{
		{"Kd", {
			mStagedBuffer->mBuffer.mData.Kd.x, 
			mStagedBuffer->mBuffer.mData.Kd.y, 
			mStagedBuffer->mBuffer.mData.Kd.z, 
			mStagedBuffer->mBuffer.mData.Kd.w
			}
		},
		{"Ks", {
			mStagedBuffer->mBuffer.mData.Ks.x, 
			mStagedBuffer->mBuffer.mData.Ks.y, 
			mStagedBuffer->mBuffer.mData.Ks.z, 
			mStagedBuffer->mBuffer.mData.Ks.w
			}
		},
		{"Ns", mStagedBuffer->mBuffer.mData.Ns},
		{"isBaseColorTextured", static_cast<int>(mStagedBuffer->mBuffer.mData.isBaseColorTextured)},

		{"BaseColorTextureUUID", mDiffuseColorTexture ? mDiffuseColorTexture->mUUIDAsset : "<empty>"},
		{"isNormalTextured", static_cast<int>(mStagedBuffer->mBuffer.mData.isNormalTextured)},

		{"NormalStrength", mStagedBuffer->mBuffer.mData.NormalStrength},
		{"NormalTextureUUID", mNormalMapTexture ? mNormalMapTexture->mUUIDAsset : "<empty>"}
	};

	return j;
}


void CoreResourceMaterial::Deserialize(const json& j)
{
	// Diffuse Color Constant
	j.at("Kd")[0].get_to(mStagedBuffer->mBuffer.mData.Kd.x);
	j.at("Kd")[1].get_to(mStagedBuffer->mBuffer.mData.Kd.y);
	j.at("Kd")[2].get_to(mStagedBuffer->mBuffer.mData.Kd.z);
	j.at("Kd")[3].get_to(mStagedBuffer->mBuffer.mData.Kd.w);

	// Specular Color Constant
	j.at("Ks")[0].get_to(mStagedBuffer->mBuffer.mData.Ks.x);
	j.at("Ks")[1].get_to(mStagedBuffer->mBuffer.mData.Ks.y);
	j.at("Ks")[2].get_to(mStagedBuffer->mBuffer.mData.Ks.z);
	j.at("Ks")[3].get_to(mStagedBuffer->mBuffer.mData.Ks.w);

	// Shiniess
	j.at("Ns").get_to(mStagedBuffer->mBuffer.mData.Ns);

	mStagedBuffer->mBuffer.mData.isBaseColorTextured = j.at("isBaseColorTextured").get<int>() == 1;
	mStagedBuffer->mBuffer.mData.isNormalTextured = j.at("isNormalTextured").get<int>() == 1;
	j.at("NormalStrength").get_to(mStagedBuffer->mBuffer.mData.NormalStrength);
}

CoreResourceMaterial::~CoreResourceMaterial()
{

}



// handle override methods of BaseResource
void CoreResourceMaterial::Upload(ID3D12GraphicsCommandList* commandList)
{
};

void CoreResourceMaterial::RegisterMainHandle(BaseMainHeap* heap)
{ 
	mStagedBuffer->RegisterMainHandle(heap);
	mMainHandle = *mStagedBuffer->mBuffer.GetHandle();
};
void CoreResourceMaterial::RegisterRuntimeHandle(BaseRuntimeHeap* heap)
{
	mStagedBuffer->RegisterRuntimeHandle(heap);
	mRuntimeHandle = mStagedBuffer->GetRuntimeHandle();
};

void CoreResourceMaterial::SetDiffuseColorTextured(BaseResourceImage* diffuseColorTexture)
{
	mStagedBuffer->mBuffer.mData.isBaseColorTextured = true;
	mDiffuseColorTexture = diffuseColorTexture;
}

void CoreResourceMaterial::SetNormalTextured(BaseResourceImage* normalMapTexture)
{
	mStagedBuffer->mBuffer.mData.isNormalTextured = true;
	mNormalMapTexture = normalMapTexture;
}

void CoreResourceMaterial::SetNormalStrength(float val)
{
	mStagedBuffer->mBuffer.mData.NormalStrength = val;
}

void CoreResourceMaterial::ReleaseMainHandle(BaseMainHeap* heap)
{
	// Remark: the handle of the BaseStagedBuffer would be released
	//         automatically when it is deleted
	//         we do not manually release here.
}