cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
	float3 gLightDir;
};

struct VertexIn
{
	float3 Pos   : POSITION;
	float4 Color : COLOR;
	float3 Norm  : NORMAL;
};

struct VertexOut
{
	float4 Pos  : SV_POSITION;
	float3 Norm : NORMAL;
	float4 Color : COLOR;
};


VertexOut main(VertexIn vin)
{
	VertexOut vout;

  vout.Color = vin.Color;
	vout.Pos = mul(float4(vin.Pos, 1.0f), gWorldViewProj);
	vout.Norm = vin.Norm;

	return vout;
}