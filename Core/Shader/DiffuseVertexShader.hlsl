cbuffer cbTransform : register(b0)
{
	float4x4 gWorld;
	float4x4 gRSInvT;
};

cbuffer cbCamera : register(b1)
{
	float4x4 gViewProj;
	float4 gEyePos;
	float4 gWindowSize;
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
	float4 PosW : POSITION;
	float3 Norm : NORMAL;
	float2 Coord: TEXCOORD;
};

VertexOut main(VertexIn vin)
{
	VertexOut ret;
	ret.PosW = mul(float4(vin.Pos, 1.0f), gWorld);
	ret.Pos = mul(ret.PosW, gViewProj);

	ret.Norm = normalize(mul(vin.Norm, gRSInvT));
	ret.Coord = vin.Coord;
	return ret;
}