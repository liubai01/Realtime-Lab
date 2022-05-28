#include "BaseAssetNode.h"
#include "../BaseUUIDGenerator.h"
#include <filesystem>
#include <type_traits>
#include <fstream>

template <typename E>
constexpr auto to_underlying(E e) noexcept
{
	return static_cast<std::underlying_type_t<E>>(e);
}

using namespace std;

int dirExists(const std::string& path)
{
	struct stat info;

	if (stat(path.c_str(), &info) != 0)
		return 0;
	else if (info.st_mode & S_IFDIR)
		return 1;
	else
		return 0;
}

inline bool fileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}


BaseAssetNode::BaseAssetNode(const string& path, BaseAssetNode* parent)
{
	mParentAsset = parent;
	mSubAssets.clear();
	mType = BaseAssetType::ASSET_UNKNOWN;
	mUUID = uuid::generate_uuid();
	mFullPath = path;
}


string BaseAssetNode::GetRelativePath()
{
	string ret = "";

	if (mParentAsset->mType != BaseAssetType::ASSET_ROOT) {
		ret = mParentAsset->GetRelativePath() + "\\";
	}

	return ret + mID;
}

BaseAssetNode* BaseAssetNode::SearchByID(const string ID)
{
	for (unique_ptr<BaseAssetNode>& node: mSubAssets)
	{
		if (node->mID == ID)
		{
			return node.get();
		}
	}

	return nullptr;
}

BaseAssetType BaseAssetNode::GetAssetType()
{
	return mType;
}

void BaseAssetNode::SetAssetType(BaseAssetType astType)
{
	mType = astType;
}

json BaseAssetNode::Serialize()
{
	json ret = json{ 
		{"AssetType", to_underlying(mType)}, 
		{"UUID", mUUID}
	};

	return ret;
}

void BaseAssetNode::Deserialize(const json& j)
{
	j.at("AssetType").get_to(mType);
	j.at("UUID").get_to(mUUID);
}

BaseAssetNode* BaseAssetNode::RegisterAsset(const string path)
{
	unique_ptr<BaseAssetNode> ret = nullptr;
	string ID = path.substr(path.find_last_of("/\\") + 1);

	// the file not exists, return null directly
	if (!fileExists(path))
	{
		return nullptr;
	}

	ret = make_unique<BaseAssetNode>(path, this);
	ret->mID = ID;

	string pathMeta = path + ".asset";

	// load the metadata if we have assigned it before
	if (filesystem::exists(pathMeta))
	{
		// load meta
		std::ifstream i(pathMeta);
		json j;
		i >> j;
		ret->Deserialize(j);
	}
	else {
		// dump meta
		// if the path points to a directory, register it
		if (dirExists(path)) {
			ret->mType = BaseAssetType::ASSET_FOLDER;
			mSubAssets.push_back(move(ret));
			return mSubAssets.back().get();
		}

		// if program reaches here, it means that it is a file,
		// we decide its type by its extension postfix
		string postfix = ID.substr(ID.find_last_of(".") + 1);

		// wavefront model
		if (postfix == "obj")
		{
			ret->mType = BaseAssetType::ASSET_OBJ;
		}
		// image
		else if (postfix == "jpg" || postfix == "png") 
		{
			ret->mType = BaseAssetType::ASSET_IMAGE;
		}
		// matereial
		else if (postfix == "mat")
		{
			ret->mType = BaseAssetType::ASSET_MATERIAL;
		}
		// unknown
		else {
			ret->mType = BaseAssetType::ASSET_UNKNOWN;
		}

		std::ofstream o(pathMeta);
		o << std::setw(4) << ret->Serialize() << std::endl;
	}

	mSubAssets.push_back(move(ret));
	return mSubAssets.back().get();
}