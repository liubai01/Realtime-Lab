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
	XMFLOAT4 Kd = { 0.8f, 0.8f, 0.8f, 1.0f }; // Diffuse Color Constant
	XMFLOAT4 Ks = { 0.5f, 0.5f, 0.5f, 1.0f }; // Specular Color Constant
	float Ns = 323.999994f; // Shiniess
	int isBaseColorTextured = false;
	int isNormalTextured = false;
	float NormalStrength = 1.0f;
};