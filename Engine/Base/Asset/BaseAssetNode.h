#pragma once

#include <vector>
#include <string>
#include <memory>
#include "../BaseInterfaceSerializable.h"

enum class BaseAssetType { 
	ASSET_ROOT,
	ASSET_IMAGE,
	ASSET_OBJ,
	ASSET_FOLDER,
	ASSET_MATERIAL,
	ASSET_FONT,
	ASSET_UNKNOWN
};

class BaseAssetNode: public BaseInterfaceSerializable
{
public:
	BaseAssetNode(const std::string& path, BaseAssetNode* parent=nullptr);

	std::string mUUID; // the unique global ID
	std::string mID; // the name
	std::string mFullPath;
	
	
	BaseAssetType GetAssetType();
	void SetAssetType(BaseAssetType astType);
	void SetHidden(bool value);
	bool IsHidden();

	BaseAssetNode* SearchByID(const std::string ID);
	BaseAssetNode* RegisterAsset(const std::string path);
	std::string GetRelativePath();

	std::vector<std::unique_ptr<BaseAssetNode>> mSubAssets;

	json Serialize();
	void Deserialize(const json& j);

private:
	BaseAssetType mType;
	BaseAssetNode* mParentAsset;
	bool mIsHidden;
};

