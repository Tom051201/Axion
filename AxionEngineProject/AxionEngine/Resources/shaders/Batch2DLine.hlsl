cbuffer CameraData : register(b0) {
	float4x4 viewProjection;
};

struct VS_Input {
	float3 position : POSITION;
	float4 color : COLOR;
};

struct PS_Input {
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PS_Input VSMain(VS_Input input) {
	PS_Input output;

	output.position = mul(float4(input.position, 1.0f), viewProjection);
	output.color = input.color;

	return output;
}

float4 PSMain(PS_Input input) : SV_TARGET{
	return input.color;
}
