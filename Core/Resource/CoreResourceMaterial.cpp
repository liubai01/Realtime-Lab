#include "CoreResourceMaterial.h"

CoreResourceMaterial::CoreResourceMaterial(ID3D12Device* device, BaseAssetNode* assetNode) : BaseResource(device, assetNode)
{
	// for runtime deciding the acutal type of BaseResource base pointer
	mType = BaseResourceType::RESOURCE_MATERIAL;

	mName = "Unnamed";
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

};
void CoreResourceMaterial::RegisterRuntimeHandle(BaseRuntimeHeap* heap)
{
};