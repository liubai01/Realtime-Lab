#pragma once
#include "BaseAssetNode.h"
#include <string>


class BaseAssetMetadata
{
	// check file validity (whether it is corrupted or modified externally)
	std::string mMD5;

	virtual std::string Serialize();
	virtual void DeSerialize(const std::string& data);
};

