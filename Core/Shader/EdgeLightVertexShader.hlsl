cbuffer cbTransform : register(b0)
{
	float4x4 gWorld;
	float4x4 gRSInvT;
};

cbuffer cbCamera : register(b1)
{
	float4x4 gViewProj;
	float4 gEyePos;
};

struct VertexIn
{
	float3 Pos   : POSITION;
	float3 Norm  : NORMAL;
	float2 Coord: TEXCOORD;
};

struct VertexOut
{
	float4 Pos  : SV_POSITION;
};

VertexOut main(VertexIn vin)
{
	VertexOut ret;

	// extrude
	//ret.Pos = float4(vin.Pos + vin.Norm * 0.01f, 1.0f);
	ret.Pos = float4(vin.Pos, 1.0f);
	ret.Pos = mul(ret.Pos, gWorld);
	ret.Pos = mul(ret.Pos, gViewProj);

	return ret;
}