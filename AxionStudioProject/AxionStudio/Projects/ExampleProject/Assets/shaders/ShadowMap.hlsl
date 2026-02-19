cbuffer LightCameraBuffer : register(b0) {
	float4x4 u_lightViewProjection;
}

cbuffer ObjectBuffer : register(b1) {
	float4 color;
	float4x4 u_transform;
}

struct VS_Input {
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 texCoord : TEXCOORD;
};

float4 VSMain(VS_Input input) : SV_POSITION{
	float4 worldPosition = mul(float4(input.position, 1.0f), u_transform);
	return mul(worldPosition, u_lightViewProjection);
}
