Texture2D t1 : register(t0);
Texture2D t2 : register(t1);
SamplerState s1 : register(s0);

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
	float4x4 gWorld;
	float4x4 gRSInvT;
	float4 gLightDir;
	float4 gEyePos;
	float4x4 gShadowViewProj;
};

cbuffer matPerObject: register(b1)
{
	float4 Ka;
	float4 Kd;
	float4 Ks;
	float Ns;
	bool IsTextured;
};

struct VertexOut
{
	float4 Pos  : SV_POSITION;
	float4 PosW : POSITION;
	float3 Norm : NORMAL;
	float2 Coord: TEXCOORD;

	float4 PosSM : POSITION1;
};

float4 main(VertexOut pin) : SV_TARGET
{
	// Shadow
	float2 shadowCoord = float2(
		(pin.PosSM.x + 1.0f) / 2.0f, 
		(1.0f - pin.PosSM.y) / 2.0f
	);

	float4 N = float4(pin.Norm, 0.0f);
	float bias = max(.005 * (1 - dot(N, gLightDir)), .0005);

  float sampledDepth = t2.Sample(s1, shadowCoord) + bias;
	float transformedDepth = pin.PosSM.z / pin.PosSM.w;
	float2 nowCoord;
	float shadow = 0.0f;
	float2 shadowMapSize = float2(1024.0f, 768.0f);
	int n = 5;


	float dist = (transformedDepth - sampledDepth) / sampledDepth;
	float dialte = 2.0f;
	//dialte = clamp(dialte, 0.5f, 20.0f);

	for (int x = -n; x <= n; ++x) {
		for (int y = -n; y <= n; ++y) {
			nowCoord = shadowCoord + float2(x / shadowMapSize.x, y / shadowMapSize.y) * dialte;
			sampledDepth = t2.Sample(s1, nowCoord) + bias;
			shadow += sampledDepth > transformedDepth;

			// update dialte
			//dist = (transformedDepth - sampledDepth) / sampledDepth;
			//dialte = 3000.0f * dist * 0.01f + dialte * 0.99f;
			//dialte = clamp(dialte, 0.5f, 20.0f);
		}
	}
	shadow /= (2 * n + 1) * (2 * n + 1);
	// threshold and transparency
	float faceAway = dot(gLightDir, N) < 0;
	shadow = 1.0f - (1.0f - shadow) * (1.0f - faceAway) / 1.0f;

	//float sampledDepth = t2.Sample(s1, shadowCoord) + 0.0015f;
	//float transformedDepth = pin.PosSM.z / pin.PosSM.w;

	// light coeff (tmp)
	float4 ims = float4(1.0f, 1.0f, 1.0f, 1.0f); // intensity of specular
	float4 ia = float4(0.6f, 0.6f, 0.6f, 1.0f); // intensity of ambient
	float4 id = float4(1.0f, 1.0f, 1.0f, 1.0f); // intensity of diffuse

	float4 Fd = Kd;
	float4 Fa = Ka * Kd;
	float4 Fs = Ks;

	if (IsTextured) {
		Fd = t1.Sample(s1, pin.Coord);
		Fa = t1.Sample(s1, pin.Coord);
	}

	// Diffuse
  float4 ret = max(dot(gLightDir, N), 0.0f) * Fd * id;
	ret *= shadow;

	// Ambient
	ret += Fa * ia;

	// Specular
	float4 V = normalize(gEyePos - pin.PosW);
	float4 H = (V + gLightDir) / length(V + gLightDir);
	ret += pow(max(dot(N, H), 0), Ns) * Fs * ims * shadow;


	// Gamma Correction
	//float vis = faceAway;
	//ret.x = shadowCoord.x;
	//ret.y = shadowCoord.y;
	//ret.z = 0.0f;
	ret = pow(ret, 2.2f);

	return ret;
}