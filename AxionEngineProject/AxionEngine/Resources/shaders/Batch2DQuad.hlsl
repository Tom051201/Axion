Texture2D g_Textures[] : register(t0, space0);
SamplerState texSampler : register(s0);

cbuffer CameraData : register(b0) {
	float4x4 viewProjection;
};

struct VS_Input {
	float3 position : POSITION;
	float4 color : COLOR;
	float2 texCoord : TEXCOORD;
	float texIndex : TEXINDEX;
	float tiling : TILING;
	int entityID : ENTITY_ID;
};

struct PS_Input {
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texCoord : TEXCOORD;
	nointerpolation uint texIndex : TEXINDEX;
	float tiling : TILING;
	nointerpolation int entityID : ENTITY_ID;
};

struct PixelOutput {
	float4 color : SV_TARGET0;
	int entityID : SV_TARGET1;
};


PS_Input VSMain(VS_Input input) {
	PS_Input output;
	output.position = mul(float4(input.position, 1.0f), viewProjection);
	output.color = input.color;
	output.texCoord = input.texCoord;
	output.texIndex = (uint)(input.texIndex + 0.5f);
	output.tiling = input.tiling;
	output.entityID = input.entityID;

	return output;
}


PixelOutput PSMain(PS_Input input) {
	float2 tiledUV = input.texCoord * input.tiling;
	float4 texColor = g_Textures[NonUniformResourceIndex(input.texIndex)].Sample(texSampler, tiledUV);
	float4 finalColor = texColor * input.color;

	if (finalColor.a < 0.05f)
		discard;

	PixelOutput output;
	output.color = finalColor;
	output.entityID = input.entityID;

	return output;
}
