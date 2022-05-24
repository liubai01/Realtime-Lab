#pragma once

#include <vector>
#include <string>
#include <memory>

using namespace std;

enum class BaseAssetType { 
	ASSET_ROOT,
	ASSET_IMAGE,
	ASSET_OBJ,
	ASSET_FOLDER,
	ASSET_UNKNOWN
};

class BaseAssetNode
{
public:
	BaseAssetNode(BaseAssetNode* parent=nullptr);

	string mID;
	
	BaseAssetType GetAssetType();
	void SetAssetType(BaseAssetType astType);

	BaseAssetNode* SearchByID(const string ID);
	BaseAssetNode* RegisterAsset(const string path);
	string GetRelativePath();

	vector<unique_ptr<BaseAssetNode>> mSubAssets;
private:
	BaseAssetType mType;
	BaseAssetNode* mParentAsset;
};

