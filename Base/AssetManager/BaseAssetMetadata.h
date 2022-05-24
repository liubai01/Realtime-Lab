#pragma once
#include "BaseAssetNode.h"
#include <string>

using namespace std;

class BaseAssetMetadata
{
	// check file validity (whether it is corrupted or modified externally)
	string mMD5;

	virtual string Serialize();
	virtual void DeSerialize(const string& data);
};

