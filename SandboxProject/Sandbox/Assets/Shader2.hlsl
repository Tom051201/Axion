cbuffer CameraBuffer : register(b0) {
	float4x4 u_viewProjection;
};

cbuffer TransformBuffer : register(b1) {
	float4 u_color;
	float4x4 u_transform;
}

struct VSInput {
	float3 pos : POSITION;
	float4 col : COLOR;
};

struct PSInput {
	float4 pos : SV_POSITION;
	float4 col : COLOR;
};

PSInput VSMain(VSInput input) {
	PSInput output;
	float4 worldPos = mul(u_transform, float4(input.pos, 1.0f));
	output.pos = mul(worldPos, u_viewProjection);
	output.col = output.pos;
	return output;
}

float4 PSMain(float4 pos : SV_POSITION, float4 col : COLOR) : SV_TARGET{
	return col;
}
