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

	mBaseColorTextureUUID = "<empty>";
	mNormalMapTextureUUID = "<empty>";
	mMetallicTextureUUID = "<empty>";
	mRoughnessTextureUUID = "<empty>";
	
	Deserialize(j);
}

json CoreResourceMaterial::Serialize()
{
	json j = json{
		// Base Color
		{"isBaseColorTextured", static_cast<int>(mStagedBuffer->mBuffer.mData.isBaseColorTextured)},
		{"baseColor", {
			mStagedBuffer->mBuffer.mData.baseColor.x, 
			mStagedBuffer->mBuffer.mData.baseColor.y,
			mStagedBuffer->mBuffer.mData.baseColor.z,
			mStagedBuffer->mBuffer.mData.baseColor.w
			}
		},
		{"baseColorTextureUUID", mBaseColorTextureUUID},

		// Roughness
		{"isRoughnessTextured", static_cast<int>(mStagedBuffer->mBuffer.mData.isBaseColorTextured)},
		{"roughness", mStagedBuffer->mBuffer.mData.roughness},
		{"roughnessTextureUUID", mRoughnessTextureUUID},

		// Metallic
		{"isMetallicTextured", static_cast<int>(mStagedBuffer->mBuffer.mData.isMetallicTextured)},
		{"metallic", mStagedBuffer->mBuffer.mData.metallic},
		{"metallicTextureUUID", mMetallicTextureUUID},

		// Normal
		{"isNormalTextured", static_cast<int>(mStagedBuffer->mBuffer.mData.isNormalTextured)},
		{"normalStrength", mStagedBuffer->mBuffer.mData.normalStrength},
		{"normalTextureUUID", mNormalMapTextureUUID}
	};

	return j;
}

void CoreResourceMaterial::Deserialize(const json& j)
{
	// Base Color
	mStagedBuffer->mBuffer.mData.isBaseColorTextured = j.at("isBaseColorTextured").get<int>() == 1;
	mBaseColorTextureUUID = j.at("baseColorTextureUUID").get<std::string>();
	j.at("baseColor")[0].get_to(mStagedBuffer->mBuffer.mData.baseColor.x);
	j.at("baseColor")[1].get_to(mStagedBuffer->mBuffer.mData.baseColor.y);
	j.at("baseColor")[2].get_to(mStagedBuffer->mBuffer.mData.baseColor.z);
	j.at("baseColor")[3].get_to(mStagedBuffer->mBuffer.mData.baseColor.w);

	// Roughness
	mStagedBuffer->mBuffer.mData.isRoughnessTextured = j.at("isRoughnessTextured").get<int>() == 1;
	mRoughnessTextureUUID = j.at("roughnessTextureUUID").get<std::string>();
	j.at("roughness").get_to(mStagedBuffer->mBuffer.mData.roughness);

	// Metallic
	mStagedBuffer->mBuffer.mData.isMetallicTextured = j.at("isMetallicTextured").get<int>() == 1;
	mMetallicTextureUUID = j.at("metallicTextureUUID").get<std::string>();
	j.at("metallic").get_to(mStagedBuffer->mBuffer.mData.metallic);
	
	// Normal
	mStagedBuffer->mBuffer.mData.isNormalTextured = j.at("isNormalTextured").get<int>() == 1;
	mNormalMapTextureUUID = j.at("normalTextureUUID").get<std::string>();
	j.at("normalStrength").get_to(mStagedBuffer->mBuffer.mData.normalStrength);
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
}
void CoreResourceMaterial::RegisterRuntimeHandle(BaseRuntimeHeap* heap)
{
	mStagedBuffer->RegisterRuntimeHandle(heap);
	mRuntimeHandle = mStagedBuffer->GetRuntimeHandle();
}

void CoreResourceMaterial::SetMetallicTextured(BaseResourceImage* metallicTexture)
{
	mStagedBuffer->mBuffer.mData.isMetallicTextured = true;
	mMetallicTextureUUID = metallicTexture->mUUIDAsset;
}

void CoreResourceMaterial::SetRoughnessTextured(BaseResourceImage* roughnessTexture)
{
	mStagedBuffer->mBuffer.mData.isRoughnessTextured = true;
	mRoughnessTextureUUID = roughnessTexture->mUUIDAsset;
}

void CoreResourceMaterial::SetBaseColorTextured(BaseResourceImage* diffuseColorTexture)
{
	mStagedBuffer->mBuffer.mData.isBaseColorTextured = true;
	mBaseColorTextureUUID = diffuseColorTexture->mUUIDAsset;
}

void CoreResourceMaterial::SetNormalTextured(BaseResourceImage* normalMapTexture)
{
	mStagedBuffer->mBuffer.mData.isNormalTextured = true;
	mNormalMapTextureUUID = normalMapTexture->mUUIDAsset;
}

void CoreResourceMaterial::SetNormalStrength(float val)
{
	mStagedBuffer->mBuffer.mData.normalStrength = val;
}

void CoreResourceMaterial::SetRoughness(float val)
{
	mStagedBuffer->mBuffer.mData.roughness = val;
}

void CoreResourceMaterial::SetMetallic(float val)
{
	mStagedBuffer->mBuffer.mData.metallic = val;
}

void CoreResourceMaterial::ReleaseMainHandle(BaseMainHeap* heap)
{
	// Remark: the handle of the BaseStagedBuffer would be released
	//         automatically when it is deleted
	//         we do not manually release here.
}