#include "BaseAssetManager.h"

BaseAssetManager::BaseAssetManager(const string assetRootDirPath)
{
	mRootPath = assetRootDirPath;

    mRootAsset = make_unique<BaseAssetNode>();
    mRootAsset->mID = "Root";
    mRootAsset->mType = BaseAssetType::ASSET_ROOT;
}

string BaseAssetManager::GetAssetFullPath(BaseAssetNode* node)
{
    return mRootPath + "\\" + node->GetRelativePath();
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
            // fallback: try to retrieve it by checking whether it exists physically
            // in file system (file system hierarchy should be the same as the asset tree)
            nowAssetNode = nowAssetNode->RegisterAsset(nowFullPath);
            
            if (!nowAssetNode)
            {
                // file/folder not exsits
                return nullptr;
            }
        }
        else {
            // this asset is found
            // if it is possibly a folder, proceed
            nowAssetNode = tmpNode;
        }

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
            // fallback: try to retrieve it by checking whether it exists physically
            // in file system (file system hierarchy should be the same as the asset tree)
            nowAssetNode = nowAssetNode->RegisterAsset(nowFullPath);

            if (!nowAssetNode)
            {
                // file/folder not exsits
                return nullptr;
            }
        }
        else {
            // this asset is found
            // it is possibly a folder, proceed
            nowAssetNode = tmpNode;
        }
    }

    return nowAssetNode;
}
