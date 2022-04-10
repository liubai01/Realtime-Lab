struct VertexOut
{
	float4 Pos  : SV_POSITION;
	float4 PosW : POSITION;
	float3 Norm : NORMAL;
};

cbuffer matPerObject: register(b2)
{
	float4 Ka;
	float4 Kd;
	float4 Ks;
	float Ns;
};

cbuffer cbCamera : register(b1)
{
	float4x4 gViewProj;
	float4 gEyePos;
};

float4 main(VertexOut pin) : SV_TARGET
{
	// light coeff (tmp)
	float4 ims = float4(1.0f, 1.0f, 1.0f, 1.0f); // intensity of specular
	float4 ia = float4(0.1f, 0.1f, 0.1f, 1.0f); // intensity of ambient
	float4 id = float4(1.0f, 1.0f, 1.0f, 1.0f); // intensity of diffuse
	float4 gLightDir = normalize(float4(1.0f, 1.0f, 1.0f, 0.0f));

	float4 N = float4(pin.Norm, 0.0f);
	float4 Fd = Kd;
	float4 Fa = Ka * Kd;
	float4 Fs = Ks;

	float4 ret = max(dot(gLightDir, N), 0.0f) * Fd * id;

	// Ambient
	ret += Fa * ia;

	// Specular
	float4 V = normalize(gEyePos - pin.PosW);
	float4 H = (V + gLightDir) / length(V + gLightDir);
	ret += pow(max(dot(N, H), 0), Ns) * Fs * ims;

	// Gamma Correction
	ret = pow(ret, 2.2f);

	return ret;
}