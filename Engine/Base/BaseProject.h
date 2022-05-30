#pragma once
#include "../ThirdParty/json.h"
#include "Asset/BaseAssetManager.h"

#include <string>

// for convenience
using json = nlohmann::json;

class BaseProject
{
public:
	BaseProject(const std::string& projectPath);
	~BaseProject();
	std::string GetProjectPath();

	BaseAssetManager* mAssetManager;
private:
	std::string mProjectPath;
};

