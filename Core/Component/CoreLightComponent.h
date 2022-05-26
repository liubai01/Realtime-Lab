#pragma once
#include "../CoreStructs.h"
#include "../../Base/Component/BaseComponent.h"

class CoreLightComponent: public BaseComponent
{
public:
	CoreLightComponent();

	void OnEditorGUI();

	void SetLightDir(float x, float y, float z);

	CoreDirectLightConsts mData;

	XMFLOAT4 mDiffuseColor;
	float mIdMag;
};

