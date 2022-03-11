cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
	float4x4 gWorld;
	float4 gLightDir;
	float4 gEyePos;
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
	float4 PosW : POSITION;
	float3 Norm : NORMAL;
	float4 Color : COLOR;
};


VertexOut main(VertexIn vin)
{
	VertexOut vout;

  vout.Color = vin.Color;
	vout.Pos = mul(float4(vin.Pos, 1.0f), gWorldViewProj);
	vout.PosW = mul(float4(vin.Pos, 1.0f), gWorld);
	vout.Norm = vin.Norm;

	return vout;
}