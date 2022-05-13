Texture2D colorTexture : register(t0);
SamplerState s0 : register(s0);

struct VertexOut
{
	float4 Pos  : SV_POSITION;
	float4 PosW : POSITION;
	float3 Norm : NORMAL;
	float2 Coord: TEXCOORD;
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
};

cbuffer lightData: register(b3)
{
	float4 ims;
	float4 ia;
	float4 id;
	float4 gLightDir;
};

float4 main(VertexOut pin) : SV_TARGET
{
	float4 diffuseCoff;
	if (isDiffuseColorTextured)
	{
		diffuseCoff = colorTexture.Sample(s0, pin.Coord);
	}
	else {
		diffuseCoff = Kd;
	}


	float4 N = float4(pin.Norm, 0.0f);
	float4 ret = max(dot(gLightDir, N), 0.0f) * diffuseCoff * id;

	// Ambient
	ret += diffuseCoff * ia;

	// Specular
	float4 V = normalize(gEyePos - pin.PosW);
	float4 H = (V + gLightDir) / length(V + gLightDir);
	ret += pow(max(dot(N, H), 0), Ns) * Ks * ims;

	// Gamma Correction
	ret = pow(ret, 2.2f);

	return ret;
}