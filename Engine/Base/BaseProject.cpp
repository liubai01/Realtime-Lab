#include "BaseProject.h"

BaseProject::BaseProject(const std::string& projectPath)
{
	mProjectPath = projectPath;
	mAssetManager = new BaseAssetManager(mProjectPath + "\\Asset");
}

BaseProject::~BaseProject()
{
	delete mAssetManager;
}

std::string BaseProject::GetProjectPath()
{
	return mProjectPath;
}