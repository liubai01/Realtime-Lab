#pragma once
#include "BaseAssetNode.h"
#include <memory>

using namespace std;

class BaseAssetManager
{
	// BaseAssetManager provides an interface of files (include metadata) in the project.
	// It is a wrapper of system filesystem and provide a serialization of the asset structure.

public:
	BaseAssetManager(const string assetRootDirPath);
	BaseAssetNode* RegisterAsset(const string url, const string filepath);
	BaseAssetNode* LoadAsset(const string url);
	string GetAssetFullPath(BaseAssetNode* node);

	unique_ptr<BaseAssetNode> mRootAsset;
	string mRootPath;
};

