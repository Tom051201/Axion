cbuffer SceneBuffer : register(b0) {
	float4x4 u_view;
	float4x4 u_projection;
	float4x4 u_viewProjection;
};

cbuffer ObjectBuffer : register(b1) {
	float4 u_color;
	float4x4 u_modelMatrix;
};

struct VSInput {
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv  : TEXCOORD;
};

struct PSInput {
	float4 pos : SV_POSITION;
	float3 worldPos : POSITION_WS;
	float4 color : COLOR;
};

PSInput VSMain(VSInput input) {
	PSInput output;

	float4 worldPos = mul(float4(input.pos, 1.0f), u_modelMatrix);
	output.worldPos = worldPos.xyz;
	output.pos = mul(worldPos, u_viewProjection);
	output.color = u_color;

	return output;
}

float4 PSMain(PSInput input) : SV_TARGET {
	return cos(input.pos * input.color);
}
