#include "BaseAssetManager.h"
#include <filesystem>


BaseAssetManager::BaseAssetManager(const std::string assetRootDirPath)
{
	mRootPath = std::filesystem::absolute(assetRootDirPath).string();

    mRootAsset = std::make_unique<BaseAssetNode>(mRootPath);
    mRootAsset->mID = "AssetRoot";
    mRootAsset->SetAssetType(BaseAssetType::ASSET_ROOT);
}

std::string BaseAssetManager::GetAssetFullPath(BaseAssetNode* node)
{
    std::string relPath = node->GetRelativePath();
    return std::filesystem::absolute(mRootPath + "\\" + relPath).string();
}

BaseAssetNode* BaseAssetManager::RegisterAsset(const std::string url, const std::string filepath)
{
    // initialize the folder of the url
    std::string delimiter = "\\";
    std::string urltail = url;

    BaseAssetNode* nowAssetNode = mRootAsset.get();
    std::string nowFullPath = mRootPath;

    size_t pos = 0;
    std::string token;
    // this loop would not include the tail filename
    while ((pos = urltail.find(delimiter)) != std::string::npos) {
        token = urltail.substr(0, pos);
        BaseAssetNode* tmpNode;
        tmpNode = nowAssetNode->SearchByID(token);
        nowFullPath += delimiter + token;
        
        // this folder has not been registered / not exists
        if (!tmpNode) {
            // fallback: try to retrieve it by checking whether it exists physically
            // in file system (file system hierarchy should be the same as the asset tree)

            if (!std::filesystem::exists(nowFullPath) && !std::filesystem::is_directory(nowFullPath))
            {
                // file/folder not exsits
                // create a folder
                if (!std::filesystem::create_directory(nowFullPath.c_str()))
                {
                    // folder fails to create
                    return nullptr;
                }
                nowAssetNode = nowAssetNode->RegisterAsset(nowFullPath);
                mUUID2AssetNode[nowAssetNode->mUUID] = nowAssetNode;
            }
            else if (std::filesystem::is_directory(nowFullPath)) {
                nowAssetNode = nowAssetNode->RegisterAsset(nowFullPath);
                mUUID2AssetNode[nowAssetNode->mUUID] = nowAssetNode;
            }
        }
        else {
            // this folder is found
            // Error: could not add an asset under a non-folder asset
            if (tmpNode->GetAssetType() != BaseAssetType::ASSET_FOLDER)
            {
                return nullptr;
            }
            nowAssetNode = tmpNode;
        }

        urltail.erase(0, pos + delimiter.length());
    }

    // the url tail goes to a file
    if (urltail.size() > 0)
    {
        // get the full path of the file
        BaseAssetNode* tmpNode;
        tmpNode = nowAssetNode->SearchByID(urltail);
        nowFullPath += delimiter + urltail;

        // this asset has been registered
        if (tmpNode) {
            return nullptr;
        }
    }

    // check whether file has already been in a proper position
    // if not copy it first
    std::error_code err;
    if (!std::filesystem::equivalent(filepath, nowFullPath, err))
    {
        std::filesystem::copy(filepath, nowFullPath);
    }

    // register the file
    nowAssetNode = nowAssetNode->RegisterAsset(nowFullPath);
    mUUID2AssetNode[nowAssetNode->mUUID] = nowAssetNode;

    return nowAssetNode;
}

BaseAssetNode* BaseAssetManager::LoadAsset(const std::string url)
{
    // traverse the url folders to the file
    std::string delimiter = "\\";
    std::string urltail = url;
    
    BaseAssetNode* nowAssetNode = mRootAsset.get();
    std::string nowFullPath = mRootPath;

    size_t pos = 0;
    std::string token;
    while ((pos = urltail.find(delimiter)) != std::string::npos) {
        token = urltail.substr(0, pos);
        BaseAssetNode* tmpNode;
        tmpNode = nowAssetNode->SearchByID(token);
        nowFullPath += delimiter + token;
        // this asset has not been registered / not exists
        if (!tmpNode) {
            return nullptr;
        }

        // this asset is found
        nowAssetNode = tmpNode;

        urltail.erase(0, pos + delimiter.length());
    }

    // the url tail goes to a file
    if (urltail.size() > 0)
    {
        // run the same logic as above, except for slicing the url std::string
        BaseAssetNode* tmpNode;
        tmpNode = nowAssetNode->SearchByID(urltail);
        nowFullPath += delimiter + urltail;
        // this asset has not been registered / not exists
        if (!tmpNode) {
            return nullptr;
        }
        nowAssetNode = tmpNode;
    }

    return nowAssetNode;
}

BaseAssetNode* BaseAssetManager::LoadAssetByUUID(const std::string uuid)
{
    auto f = mUUID2AssetNode.find(uuid);

    if (f != mUUID2AssetNode.end())
    {
        return f->second;
    }

    return nullptr;
}
