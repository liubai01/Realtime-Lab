#pragma once

#include <vector>
#include <string>
#include <memory>
#include "../BaseInterfaceSerializable.h"

using namespace std;

enum class BaseAssetType { 
	ASSET_ROOT,
	ASSET_IMAGE,
	ASSET_OBJ,
	ASSET_FOLDER,
	ASSET_MATERIAL,
	ASSET_UNKNOWN
};

class BaseAssetNode: public BaseInterfaceSerializable
{
public:
	BaseAssetNode(const string& path, BaseAssetNode* parent=nullptr);

	string mUUID; // the unique global ID
	string mID; // the name
	string mFullPath;
	
	BaseAssetType GetAssetType();
	void SetAssetType(BaseAssetType astType);

	BaseAssetNode* SearchByID(const string ID);
	BaseAssetNode* RegisterAsset(const string path);
	string GetRelativePath();

	vector<unique_ptr<BaseAssetNode>> mSubAssets;

	json Serialize();
	void Deserialize(const json& j);

private:
	BaseAssetType mType;
	BaseAssetNode* mParentAsset;
};

