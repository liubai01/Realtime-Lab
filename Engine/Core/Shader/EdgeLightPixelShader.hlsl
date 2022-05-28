struct VertexOut
{
	float4 Pos  : SV_POSITION;
};

cbuffer cbCamera : register(b1)
{
	float4x4 gViewProj;
	float4 gEyePos;
	float4 gWindowSize;
};


float4 main(VertexOut pin) : SV_TARGET
{
	return float4(1.0f, 1.0f, 0.0f, 1.0f);
}