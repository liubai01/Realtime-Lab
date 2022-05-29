#pragma once
#include "BaseAssetNode.h"
#include <memory>

class BaseAssetManager
{
	// BaseAssetManager provides an interface of files (include metadata) in the project.
	// It is a wrapper of system filesystem and provide a serialization of the asset structure.

public:
	BaseAssetManager(const std::string assetRootDirPath);
	// TBD: rewrite those api's by const& string, and const&& string)
	BaseAssetNode* RegisterAsset(const std::string url, const std::string filepath);
	BaseAssetNode* LoadAsset(const std::string url);
	BaseAssetNode* LoadAssetByUUID(const std::string uuid);

	std::string GetAssetFullPath(BaseAssetNode* node);

	std::unique_ptr<BaseAssetNode> mRootAsset;
	std::string mRootPath;

private:
	std::unordered_map<std::string, BaseAssetNode*> mUUID2AssetNode;
};

