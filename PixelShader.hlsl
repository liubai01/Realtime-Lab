cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
	float4x4 gWorld;
	float4 gLightDir;
	float4 gEyePos;
};

struct VertexOut
{
	float4 Pos  : SV_POSITION;
	float4 PosW : POSITION;
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
	ret += 0.4f * pin.Color;

	// Specular
	float3 V = normalize(gEyePos - pin.PosW);
	float3 R = 2 * dot(pin.Norm, gLightDir) * pin.Norm - gLightDir;
	ret += pow(max(dot(V, R), 0), 2.0f) * float4(1.0f, 1.0f, 1.0f, 1.0f) * 0.1f;

	ret.w = 1.0f;

	// Gamma Correction
	ret = pow(ret, 2.2f);

	return ret;
}