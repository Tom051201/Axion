cbuffer CameraData : register(b0) {
	float4x4 viewProjection;
};

Texture2D textures[16] : register(t0);
SamplerState texSampler : register(s0);

struct VS_Input {
	float3 position : POSITION;
	float4 color : COLOR;
	float2 texCoord : TEXCOORD;
	float texIndex : TEXINDEX;
	float tiling : TILING;
};

struct PS_Input {
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texCoord : TEXCOORD;
	nointerpolation float texIndex : TEXINDEX;
	float tiling : TILING;
};

PS_Input VSMain(VS_Input input) {
	PS_Input output;

	output.position = mul(float4(input.position, 1.0f), viewProjection);
	output.color = input.color;
	output.texCoord = input.texCoord;
	output.texIndex = input.texIndex;
	output.tiling = input.tiling;

	return output;
}

float4 PSMain(PS_Input input) : SV_TARGET{
	float2 tiledUV = input.texCoord * input.tiling;
	int index = (int)(input.texIndex + 0.5f);
	float4 texColor = textures[NonUniformResourceIndex(index)].Sample(texSampler, tiledUV);
	float4 finalColor = texColor * input.color;

	if (finalColor.a < 0.05f) discard;

	return finalColor;
}
