cbuffer SceneData : register(b0) {
	matrix view;
	matrix projection;
	matrix viewProjection;
	matrix lightSpaceMatrix;
};

struct VSInput {
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 texCoord : TEXCOORD;
	float4 instanceColor : COLOR;
	float4x4 instanceTransform : ROW;
};

float4 VSMain(VSInput input) : SV_POSITION {
	float4 worldPos = mul(float4(input.position, 1.0), input.instanceTransform);
    return mul(worldPos, viewProjection);
}

void PSMain() {

}