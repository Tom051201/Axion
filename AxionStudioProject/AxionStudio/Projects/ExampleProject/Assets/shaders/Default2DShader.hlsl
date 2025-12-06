cbuffer SceneBuffer : register(b0) {
	float4x4 u_view;
	float4x4 u_projection;
	float4x4 u_viewProjection;
};

cbuffer ObjectBuffer : register(b1) {
	float4   u_color;
	float4x4 u_modelMatrix;
};

Texture2D u_texture : register(t0);
SamplerState u_sampler : register(s0);

struct VSInput {
	float3 pos		: POSITION;
	float4 normal	: NORMAL;
	float2 uv		: TEXCOORD;
};

struct PSInput {
	float4 pos		: SV_POSITION;
	float2 uv		: TEXCOORD;
	float4 color	: COLOR;
};

PSInput VSMain(VSInput input) {
	PSInput output;

	float4 localPos = float4(input.pos, 1.0f);
	float4 worldPos = mul(localPos, u_modelMatrix);
	float4 clipPos = mul(worldPos, u_viewProjection);

	output.pos = clipPos;
	output.uv = input.uv;
	output.color = u_color;

	return output;
}

float4 PSMain(PSInput input) : SV_TARGET{
	float4 texColor = u_texture.Sample(u_sampler, input.uv);
	return texColor * input.color;
}
