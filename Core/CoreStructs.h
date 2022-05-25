#pragma once
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;

struct CoreDirectLightConsts {
	XMFLOAT4 Ims = { 1.0f, 1.0f, 1.0f, 1.0f }; // intensity of specular
	XMFLOAT4 Ia = { 0.5f, 0.5f, 0.5f, 1.0f }; // intensity of ambient
	XMFLOAT4 Id = { 1.0f, 1.0f, 1.0f ,1.0f }; // intensity of diffuse
	XMFLOAT4 LightDir = { 0.577f, 0.577f, 0.577f, 0.0f }; // light direction
};
