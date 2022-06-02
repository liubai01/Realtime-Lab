#pragma once
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;

struct CoreDirectLightConsts {
	XMFLOAT4 Ia = { 0.5f, 0.5f, 0.5f, 1.0f }; // intensity of ambient
	XMFLOAT4 Id = { 1.0f, 1.0f, 1.0f ,1.0f }; // intensity of diffuse
	XMFLOAT4 LightDir = { 0.577f, 0.577f, 0.577f, 0.0f }; // light direction
};

struct CoreMaterialConsts {
	int isBaseColorTextured = false;
	int isNormalTextured = false;
	int isMetallicTextured = false;
	int isRoughnessTextured = false;

	XMFLOAT4 baseColor = { 0.8f, 0.8f, 0.8f, 1.0f }; // Base Color Constant
	float normalStrength = 1.0f;
	float metallic = 1.0f;
	float roughness = 1.0f;
};