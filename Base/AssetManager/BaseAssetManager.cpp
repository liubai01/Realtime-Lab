#include "BaseAssetManager.h"
#include <filesystem>


BaseAssetManager::BaseAssetManager(const string assetRootDirPath)
{
	mRootPath = filesystem::absolute(assetRootDirPath).string();

    mRootAsset = make_unique<BaseAssetNode>(mRootPath);
    mRootAsset->mID = "AssetRoot";
    mRootAsset->SetAssetType(BaseAssetType::ASSET_ROOT);
}

string BaseAssetManager::GetAssetFullPath(BaseAssetNode* node)
{
    string relPath = node->GetRelativePath();
    return filesystem::absolute(mRootPath + "\\" + relPath).string();
}

BaseAssetNode* BaseAssetManager::RegisterAsset(const string url, const string filepath)
{
    // initialize the folder of the url
    string delimiter = "\\";
    string urltail = url;

    BaseAssetNode* nowAssetNode = mRootAsset.get();
    string nowFullPath = mRootPath;

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

            if (!filesystem::exists(nowFullPath) && !filesystem::is_directory(nowFullPath))
            {
                // file/folder not exsits
                // create a folder
                if (!filesystem::create_directory(nowFullPath.c_str()))
                {
                    // folder fails to create
                    return nullptr;
                }
                nowAssetNode = nowAssetNode->RegisterAsset(nowFullPath);
            }
            else if (filesystem::is_directory(nowFullPath)) {
                nowAssetNode = nowAssetNode->RegisterAsset(nowFullPath);
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
    error_code err;
    if (!filesystem::equivalent(filepath, nowFullPath, err))
    {
        filesystem::copy(filepath, nowFullPath);
    }

    // register the file
    nowAssetNode = nowAssetNode->RegisterAsset(nowFullPath);

    return nowAssetNode;
}

BaseAssetNode* BaseAssetManager::LoadAsset(const string url)
{
    // traverse the url folders to the file
    string delimiter = "\\";
    string urltail = url;
    
    BaseAssetNode* nowAssetNode = mRootAsset.get();
    string nowFullPath = mRootPath;

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
        // run the same logic as above, except for slicing the url string
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
