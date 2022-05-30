Texture2D t0 : register(t0);
SamplerState s0 : register(s0);

cbuffer cbCamera : register(b0)
{
	float4x4 gViewProj;
	float4 gEyePos;
	float4 gWindowSize;
};

struct VertexOut
{
	float4 Pos  : SV_POSITION;
};


float4 main(VertexOut pin) : SV_TARGET
{
	float2 shadowMapSize = float2(500.0f, 500.0f);
	float2 coord = float2(
		pin.Pos.x / gWindowSize.x,
		pin.Pos.y / gWindowSize.y
	);
	int n = 3;
	float4 ret = { 0.0f, 0.0f, 0.0f, 0.0f };
	int dialte = 1.5f;

	for (int x = -n; x <= n; ++x) {
		for (int y = -n; y <= n; ++y) {
			float2 nowCoord = coord + float2(x / gWindowSize.x, y / gWindowSize.y) * dialte;
			ret += sqrt(t0.Sample(s0, nowCoord));
		}
	}
	ret /= (2 * n + 1) * (2 * n + 1);
	ret += (ret > 0) * 0.5;

	ret = min(ret, 1.0f);
	
	return ret - t0.Sample(s0, coord);
}