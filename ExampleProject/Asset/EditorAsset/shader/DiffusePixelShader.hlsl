Texture2D colorTexture : register(t0);
Texture2D normalMapTexture : register(t1);
SamplerState s0 : register(s0);

struct VertexOut
{
	float4 Pos  : SV_POSITION;
	float4 PosW : POSITION;
	float3 Norm : NORMAL;
	float2 Coord: TEXCOORD;
	float3 Tan  : TANGENT;
	float3 BiTan : BITANGENT;
};

cbuffer cbCamera : register(b1)
{
	float4x4 gViewProj;
	float4 gEyePos;
	float4 gWindowSize;
};

cbuffer matPerObject: register(b2)
{
	float4 Kd;
	float4 Ks;
	float Ns;
	bool isDiffuseColorTextured;
	bool isNormalTextured;
	float NormalStrength;
};

cbuffer lightData: register(b3)
{
	float4 ia;
	float4 id;
	float4 gLightDir;
};

float4 main(VertexOut pin) : SV_TARGET
{
	float4 diffuseCoff = Kd;
	if (isDiffuseColorTextured)
	{
		diffuseCoff = colorTexture.Sample(s0, pin.Coord);
		// Gamma correction
		diffuseCoff = pow(diffuseCoff, 2.2);
	}

	float4 N = float4(pin.Norm, 0.0f);
	if (isNormalTextured)
	{
		float4 np = normalMapTexture.Sample(s0, pin.Coord) * 2.0f - 1.0f;
		float3 N3 = float3(0.0f, 0.0f, 0.0f);
		N3 += np.x * pin.Tan;
		N3 += np.y * pin.BiTan;
		N3 += np.z * pin.Norm;
		N3 = normalize(N3);
		N3 -= pin.Norm;
		N3 = pin.Norm + NormalStrength * N3;
		N = float4(normalize(N3), 0.0f);
	}

	// Diffuse
	float4 ret = max(dot(gLightDir, N), 0.0f) * diffuseCoff * id;

	// Ambient
	ret += diffuseCoff * ia;

	// Specular
	float4 V = normalize(gEyePos - pin.PosW);
	float4 H = (V + gLightDir) / length(V + gLightDir);
	ret += pow(max(dot(N, H), 0), Ns) * Ks;

	// Gamma encoding
	ret = pow(ret, 0.45);

	return ret;
}