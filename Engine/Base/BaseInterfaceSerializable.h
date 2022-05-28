#pragma once

#include "../ThirdParty/json.h"

using json = nlohmann::json;

class BaseInterfaceSerializable
{
public:
	virtual json Serialize() = 0;
	virtual void Deserialize(const json& j) = 0;
};

