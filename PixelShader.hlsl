Texture2D t1 : register(t0);
SamplerState s1 : register(s0);

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
	float4x4 gWorld;
	float4x4 gRSInvT;
	float4 gLightDir;
	float4 gEyePos;
	float4 gTexMask;
};

struct VertexOut
{
	float4 Pos  : SV_POSITION;
	float4 PosW : POSITION;
	float3 Norm : NORMAL;
	float4 Color : COLOR;
	float2 Coord: TEXCOORD;
};

float4 main(VertexOut pin) : SV_TARGET
{
	// Gamma Correction Coefficient
	float gamma = 2.2;

	// Diffuse
  float4 diffuseColor = t1.Sample(s1, pin.Coord) * gTexMask + (1.0f - gTexMask) * pin.Color;
  float4 ret = max(dot(gLightDir, pin.Norm), 0.0f) * diffuseColor * 1.2f;

	// Ambient
	ret += 0.4f * diffuseColor;

	// Specular
	float3 V = normalize(gEyePos - pin.PosW);
	float3 R = 2 * dot(pin.Norm, gLightDir) * pin.Norm - gLightDir;
	//ret = float4(0.0f, 0.0f, 0.0f, 1.0f);
	ret += pow(max(dot(V, R), 0), 2.0f) * float4(1.0f, 1.0f, 1.0f, 1.0f) * 0.3f;
	
	//ret = max(dot(V, R), 0);
	//ret = float4(R, 1.0f);
	//ret = gTexMask;
	ret.w = 1.0f;

	// Gamma Correction
	ret = pow(ret, 2.2f);

	return ret;
}