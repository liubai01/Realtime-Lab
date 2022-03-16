cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
	float4x4 gWorld;
	float4x4 gRSInvT;
	float4 gLightDir;
	float4 gEyePos;
	float4x4 gShadowViewProj;
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

	float4 PosSM : POSITION1;
};


VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.Pos = mul(float4(vin.Pos, 1.0f), gWorldViewProj);

	vout.PosW = mul(float4(vin.Pos, 1.0f), gWorld);
	vout.PosSM = mul(vout.PosW, gShadowViewProj);
	vout.PosSM /= vout.PosSM.w;
	vout.Norm = normalize(mul(vin.Norm, gRSInvT));
	vout.Coord = vin.Coord;

	return vout;
}