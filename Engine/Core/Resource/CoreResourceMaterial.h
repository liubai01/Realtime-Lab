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

	std::string mDiffuseColorTextureUUID;
	std::string mNormalMapTextureUUID;

	CoreResourceMaterial(ID3D12Device* device, BaseAssetNode* assetNode);
	~CoreResourceMaterial();

	void SetDiffuseColorTextured(BaseResourceImage* diffuseColorTexture);
	void SetNormalTextured(BaseResourceImage* normalMapTexture);
	void SetNormalStrength(float val);

	void ReleaseMainHandle(BaseMainHeap* heap);

	json Serialize();
	void Deserialize(const json& j);

	static void CreateEmpty(const std::string& path)
	{
		json j = json{ 
			{"Kd", {0.8f, 0.8f, 0.8f, 1.0f}},
			{"Ks", {0.5f, 0.5f, 0.5f, 1.0f}},
			{"Ns", 324.0f},
			{"isBaseColorTextured", 0},
			{"isNormalTextured", 0},
			{"NormalStrength", 1.0f}
		};

		std::ofstream o(path);
		o << std::setw(4) << j << std::endl;
	}

	// handle override methods of BaseResource
	void Upload(ID3D12GraphicsCommandList* commandList);
	void RegisterMainHandle(BaseMainHeap* heap);
	void RegisterRuntimeHandle(BaseRuntimeHeap* heap);
};
