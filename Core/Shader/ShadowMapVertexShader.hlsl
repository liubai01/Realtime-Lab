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

float4 main(VertexIn vin) : SV_POSITION
{
	float4 ret = mul(float4(vin.Pos, 1.0f), gWorld);
	ret = mul(ret, gViewProj);
	return ret;
}