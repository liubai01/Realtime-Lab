cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
};

struct VertexIn
{
	float3 Pos   : POSITION;
	float4 Color : COLOR;
};

struct VertexOut
{
	float4 Pos  : SV_POSITION;
	float4 Color : COLOR;
};


VertexOut main(VertexIn vin)
{
	VertexOut vout;

  vout.Color = vin.Color;
	vout.Pos = mul(float4(vin.Pos, 1.0f), gWorldViewProj);

	return vout;
}