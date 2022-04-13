struct VertexOut
{
	float4 Pos  : SV_POSITION;
	float4 PosW : POSITION;
	float3 Norm : NORMAL;
};

cbuffer cbCamera : register(b1)
{
	float4x4 gViewProj;
	float4 gEyePos;
};

cbuffer matPerObject: register(b2)
{
	float4 Kd;
	float4 Ks;
	float Ns;
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
	float4 N = float4(pin.Norm, 0.0f);
	float4 ret = max(dot(gLightDir, N), 0.0f) * Kd * id;

	// Ambient
	ret += Kd * ia;

	// Specular
	float4 V = normalize(gEyePos - pin.PosW);
	float4 H = (V + gLightDir) / length(V + gLightDir);
	ret += pow(max(dot(N, H), 0), Ns) * Ks * ims;

	// Gamma Correction
	ret = pow(ret, 2.2f);

	return ret;
}