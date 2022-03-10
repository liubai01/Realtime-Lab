cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
	float3 gLightDir;
};

struct VertexOut
{
	float4 Pos  : SV_POSITION;
	float3 Norm : NORMAL;
	float4 Color : COLOR;
};

float4 main(VertexOut pin) : SV_TARGET
{
	// Gamma Correction Coefficient
	float gamma = 2.2;

	// Diffuse
  float4 ret = max(dot(gLightDir, pin.Norm), 0.0f) * pin.Color;

	// Ambient
	ret += 0.2f * pin.Color;

	// Specular

	// Gamma Correction
	ret = pow(ret, 2.2f);

	return ret;
}