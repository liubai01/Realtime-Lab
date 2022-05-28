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
	ret.Pos = float4(vin.Pos, 1.0f);
	return ret;
}