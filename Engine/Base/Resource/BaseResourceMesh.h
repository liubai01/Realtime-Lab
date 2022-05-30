#pragma once
#include "BaseResource.h"
#include "../../ThirdParty/tiny_obj_loader.h"
#include "../BaseGeometry.h"
#include <vector>

struct BaseResourceVertex {
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT2 texCoord;
};


class BaseResourceGeometry : public BaseGeometry<BaseResourceVertex>
{

};

class BaseResourceMesh : public BaseResource
{
public:
	static const BaseResourceType ClassResourceType = BaseResourceType::RESOURCE_MESH;
	static const BaseAssetType ClassAssetType = BaseAssetType::ASSET_OBJ;

	BaseResourceMesh(ID3D12Device* device, BaseAssetNode* assetNode);
	~BaseResourceMesh();
	
	std::vector<BaseResourceGeometry> mGeos;

	// handle override methods of BaseResource
	void Upload(ID3D12GraphicsCommandList* commandList) { };
	void RegisterMainHandle(BaseMainHeap* heap) { };
	void RegisterRuntimeHandle(BaseRuntimeHeap* heap) { };
};

