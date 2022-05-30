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
	float3 Tan   : TANGENT;
	float3 BiTan : BITANGENT;
};

struct VertexOut
{
	float4 Pos  : SV_POSITION;
	float4 PosW : POSITION;
	float3 Norm : NORMAL;
	float2 Coord: TEXCOORD;
	float3 Tan  : TANGENT;
	float3 BiTan : BITANGENT;
};

VertexOut main(VertexIn vin)
{
	VertexOut ret;
	ret.PosW = mul(float4(vin.Pos, 1.0f), gWorld);
	ret.Pos = mul(ret.PosW, gViewProj);

	ret.Norm = normalize(mul(float4(vin.Norm, 1.0f), gRSInvT));
	ret.Coord = vin.Coord;

	ret.Tan = normalize(mul(float4(vin.Tan, 1.0f), gRSInvT));
	ret.BiTan = normalize(mul(float4(vin.BiTan, 1.0f), gRSInvT));

	return ret;
}