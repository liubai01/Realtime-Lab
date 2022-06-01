Texture2D colorTexture : register(t0);
Texture2D normalMapTexture : register(t1);
Texture2D metallicTexture : register(t2);
Texture2D roughnessTexture : register(t3);
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
	bool isDiffuseColorTextured;
	bool isNormalTextured;
	bool isMetallicTextured;
	bool isRoughnessTextured;

	float4 baseColor;
	float normalStrength;
	float kMetallic;
	float kRoughness;
};

cbuffer lightData: register(b3)
{
	float4 ia;
	float4 id;
	float4 gLightDir;
};

float4 main(VertexOut pin) : SV_TARGET
{
	// phong model now (TBD: cook-torrence ggx)
	float4 diffuseCoeff = baseColor;
	float shininess = 355.0f;
	float4 specularCoeff = float4(0.2f, 0.2f ,0.2f, 1.0f);

	float metallicCoeff = kMetallic;
	if (isMetallicTextured)
	{
		metallicCoeff = metallicTexture.Sample(s0, pin.Coord);
	}

	float roughnessCoeff = kRoughness;
	if (roughnessCoeff)
	{
		roughnessCoeff = roughnessTexture.Sample(s0, pin.Coord);
	}

	if (isDiffuseColorTextured)
	{
		diffuseCoeff = colorTexture.Sample(s0, pin.Coord);
		// Gamma correction
		diffuseCoeff = pow(diffuseCoeff, 2.2);
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
		N3 = pin.Norm + normalStrength * N3;
		N = float4(normalize(N3), 0.0f);
	}

	// Diffuse
	float4 ret = max(dot(gLightDir, N), 0.0f) * diffuseCoeff * id;

	// Ambient
	ret += diffuseCoeff * ia;

	// Specular
	float4 V = normalize(gEyePos - pin.PosW);
	float4 H = (V + gLightDir) / length(V + gLightDir);
	ret += pow(max(dot(N, H), 0), shininess) * specularCoeff;

	// Gamma encoding
	ret = pow(ret, 0.45);

	return ret;
}