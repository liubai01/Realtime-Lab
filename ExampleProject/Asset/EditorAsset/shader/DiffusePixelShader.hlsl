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
	bool isBaseColorTextured;
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

float DistributionGGX(float4 N, float4 H, float a)
{
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = 3.1415926f * denom * denom;

	return nom / denom;
}

// Approximates the fresnel effect
// refer to: https://en.wikipedia.org/wiki/Schlick%27s_approximation
float4 fresnel_schlick(float NVdot, float4 baseColor, float metallic)
{
	// F0 of common dielectrics takes 0.04 as approximation
	// F = F0 + (1 - F0) * (1 - VHdot) ** 5
	float4 F = 0.04 + 0.96 * pow(1 - NVdot, 5);
	F.w = 1.0f;
	return lerp(F, baseColor, metallic);
}

// Refer to: https://learnopengl.com/PBR/Theory
float GeometrySchlickGGX(float NdotV, float k)
{
	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(float NdotV, float NdotL, float k)
{
	float ggx1 = GeometrySchlickGGX(NdotV, k);
	float ggx2 = GeometrySchlickGGX(NdotL, k);

	return ggx1 * ggx2;
}


float4 main(VertexOut pin) : SV_TARGET
{
	// phong model now (TBD: cook-torrence ggx)
	float shininess = 355.0f;
	float4 specularCoeff = float4(0.2f, 0.2f ,0.2f, 1.0f);

	float metallicCoeff = kMetallic;
	if (isMetallicTextured)
	{
		metallicCoeff = metallicTexture.Sample(s0, pin.Coord).r;
	}

	float roughnessCoeff = kRoughness;
	if (isRoughnessTextured)
	{
		roughnessCoeff = roughnessTexture.Sample(s0, pin.Coord).r;
	}

	float4 baseColorCoeff = baseColor;
	if (isBaseColorTextured)
	{
		baseColorCoeff = colorTexture.Sample(s0, pin.Coord);
		// Gamma correction
		baseColorCoeff = pow(baseColorCoeff, 2.2);
	}

	float4 N = float4(normalize(pin.Norm), 0.0f);
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

	// Cook torrance

	// Part I, D: Distribution term (Trowbridge-Reitz GGX)
	// https://learnopengl.com/PBR/Theory
	float4 V = normalize(gEyePos - pin.PosW);
	float4 H = (V + gLightDir) / length(V + gLightDir);
	float D = DistributionGGX(N, H, roughnessCoeff);


	// Part II, F: Fresnel term
	float NdotV = max(dot(N, V), 0.0f);
	float NdotL = max(dot(N, gLightDir), 0.0f);
	float4 F = fresnel_schlick(NdotV, baseColorCoeff, metallicCoeff);

	// Part III, G: Geometry term
	// k_direct is taken as https://learnopengl.com/PBR/Theory recommended
	float G = GeometrySmith(NdotV, NdotL, pow(roughnessCoeff + 1, 2) / 8);
	
	float4 BRDFCookTorrance = D * F * G / (4 * NdotV * NdotL + 0.0001f);
	float4 BRDFLambert = baseColorCoeff / 3.1415926f;

	// Merge Specular(CookTorrance) and Lambert Term 
	// http://www.codinglabs.net/article_physically_based_rendering_cook_torrance.aspx
	// BRDF = ks * BRDFCookTorrance + kd * BRDFLambert
	// ks = F and is included in BRDFCookTorrance
	// kd = 1 - ks = 1 - F

	float4 ret = BRDFCookTorrance + (1 - F) * (1 - metallicCoeff) * BRDFLambert;
	ret = ret * NdotL * id + ia * baseColorCoeff;

	// Gamma encoding
	ret = pow(ret, 0.45);

	return ret;
}