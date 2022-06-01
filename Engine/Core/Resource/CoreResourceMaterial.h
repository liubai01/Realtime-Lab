#pragma once

#include "../../Base/Resource/BaseResource.h"
#include "../../Base/Resource/BaseResourceImage.h"
#include "../CoreStructs.h"
#include "../../Base/BaseStagedBuffer.h"
#include <fstream>

class CoreResourceMaterial : public BaseResource, public BaseInterfaceSerializable
{
public:
	static const BaseResourceType ClassResourceType = BaseResourceType::RESOURCE_MATERIAL;
	static const BaseAssetType ClassAssetType = BaseAssetType::ASSET_MATERIAL;

	std::unique_ptr<BaseStagedBuffer<CoreMaterialConsts>> mStagedBuffer;

	//BaseResourceImage* mDiffuseColorTexture;
	//BaseResourceImage* mNormalMapTexture;

	std::string mBaseColorTextureUUID;
	std::string mNormalMapTextureUUID;
	std::string mMetallicTextureUUID;
	std::string mRoughnessTextureUUID;

	CoreResourceMaterial(ID3D12Device* device, BaseAssetNode* assetNode);
	~CoreResourceMaterial();

	void SetBaseColorTextured(BaseResourceImage* diffuseColorTexture);
	void SetMetallicTextured(BaseResourceImage* metallicTexture);
	void SetRoughnessTextured(BaseResourceImage* roughnessTexture);
	void SetNormalTextured(BaseResourceImage* normalMapTexture);

	void SetNormalStrength(float val);
	void SetRoughness(float val);
	void SetMetallic(float val);

	void ReleaseMainHandle(BaseMainHeap* heap);

	json Serialize();
	void Deserialize(const json& j);

	static void CreateEmpty(const std::string& path)
	{
		json j = json{ 
			// base color
			{"isBaseColorTextured", 0},
			{"baseColor", {0.8f, 0.8f, 0.8f, 1.0f}},
			{"baseColorTextureUUID", "<empty>"},
			// normal
			{"isNormalTextured", 0},
			{"normalTextureUUID", "<empty>"},
			{"normalStrength", 1.0f},
			// metallic
			{"isMetallicTextured", 0},
			{"metallic", 1.0f},
			{"metallicTextureUUID", "<empty>"},
			// roughness
			{"isRoughnessTextured", 0},
			{"roughness", 1.0f},
			{"roughnessTextureUUID", "<empty>"},
		};

		std::ofstream o(path);
		o << std::setw(4) << j << std::endl;
	}

	// handle override methods of BaseResource
	void Upload(ID3D12GraphicsCommandList* commandList);
	void RegisterMainHandle(BaseMainHeap* heap);
	void RegisterRuntimeHandle(BaseRuntimeHeap* heap);
};
