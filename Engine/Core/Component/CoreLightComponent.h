#pragma once
#include "../CoreStructs.h"
#include "../../Base/Component/BaseComponent.h"

class CoreLightComponent: public BaseComponent
{
public:
	CoreLightComponent();
	// Invoke to setup Dear ImGui
	void OnEditorGUI();
	// TBD: only support directional light here
	void SetLightDir(float x, float y, float z);

	// TBD: remove this mData structure, it is temporarily for message transportation
	// between manager and component
	CoreDirectLightConsts mData;

	// Directional light color & intensity (blinn-phong model here)
	XMFLOAT4 mDiffuseColor; // the color of the light
	float mLightIntensity; // magnitute (intensity) of the light

	// Serailization
	json Serialize();
	void Deserialize(const json& j);
};

