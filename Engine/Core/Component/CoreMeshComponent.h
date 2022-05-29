#pragma once
#include "../../Base/Component/BaseMesh.h"
#include "../../Base/Component/BaseComponent.h"
#include "../CoreGeometry.h"
#include "../Resource/CoreResourceMaterial.h"
#include "../../Base/Resource/BaseResourceMesh.h"
#include "../../Base/Asset/BaseAssetManager.h"

typedef enum class CoreMeshComponentType
{
	// the mesh component is associated with an asset
	ASSET_COMPONENT, 
	// the mesh component is associated with a primitive (sphere, plane, etc.)
	PRIMITIVE_COMPONENT,
	// the default component before a resource is associated with this component
	UNKNOWN_COMPONENT
} CoreMeshComponentType;


class CoreMeshComponent : public BaseComponent
{
public:
	// identify the source of this mesh (e.g: asset, primitive, procedural, etc.)
	CoreMeshComponentType mMeshType;
	// the UUID of associated asset / id for the primitive (sphere, plane) / ...
	std::string mID;

	std::vector<std::shared_ptr<CoreGeometry>> mGeo;
	std::vector<CoreResourceMaterial*> mMat;
	std::vector<std::unique_ptr<BaseMesh>> mMesh;

	bool mUploaded;

	CoreMeshComponent();
	// invoke in the editor to construct Dear ImGUI
	void OnEditorGUI(BaseAssetManager* assetManager, BaseResourceManager* resourceManager);
	
	void ClearGeometry();
	void AddGeometry(std::shared_ptr<CoreGeometry> geo, CoreResourceMaterial* mat=nullptr);

	void Upload(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
	void Render(ID3D12GraphicsCommandList* commandList, int matRegIdx=-1);

	// Serailization
	json Serialize();
	void Deserialize(const json& j);
};

