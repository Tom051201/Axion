cbuffer SceneBuffer : register(b0) {
	float4x4 u_viewProjection;
};

cbuffer ObjectBuffer : register(b1) {
	float4 u_color;
	float4x4 u_modelMatrix;
};

struct VSInput {
	float3 pos : POSITION;
	float4 col : COLOR;
	float2 uv  : TEXCOORD;
};

struct PSInput {
	float4 pos : SV_POSITION;
	float4 col : COLOR;
	float2 uv  : TEXCOORD;
};

PSInput VSMain(VSInput input) {
	PSInput output;
	float4 worldPos = mul(float4(input.pos, 1.0f), u_modelMatrix);
	output.pos = mul(worldPos, u_viewProjection);
	output.col = u_color;
	output.uv = input.uv;
	return output;
}

float4 PSMain(float4 pos : SV_POSITION, float4 col : COLOR) : SV_TARGET{
	return col;
}
