#include "BaseProject.h"

BaseProject::BaseProject(const string& projectPath)
{
	mProjectPath = projectPath;
	mAssetManager = new BaseAssetManager(mProjectPath + "\\Asset");
}

BaseProject::~BaseProject()
{
	delete mAssetManager;
}

string BaseProject::GetProjectPath()
{
	return mProjectPath;
}