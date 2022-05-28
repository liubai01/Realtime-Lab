#pragma once
#include "../ThirdParty/json.h"
#include "Asset/BaseAssetManager.h"

#include <string>

// for convenience
using json = nlohmann::json;
using namespace std;

class BaseProject
{
public:
	BaseProject(const string& projectPath);
	~BaseProject();
	string GetProjectPath();

	BaseAssetManager* mAssetManager;
private:
	string mProjectPath;
};

