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

struct PixelOutput {
	float4 color : SV_TARGET0;
	int entityID : SV_TARGET1;
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

//PixelOutput PSMain(PS_Input input) {
//	float2 tiledUV = input.texCoord * input.tiling;
//	int index = (int)(input.texIndex + 0.5f);
//	float4 texColor = textures[NonUniformResourceIndex(index)].Sample(texSampler, tiledUV);
//	float4 finalColor = texColor * input.color;
//
//	if (finalColor.a < 0.05f) discard;
//
//	PixelOutput output;
//	output.color = finalColor;
//	output.entityID = -1;
//
//	return output;
//}

PixelOutput PSMain(PS_Input input) {
	float2 tiledUV = input.texCoord * input.tiling;
	int index = (int)(input.texIndex + 0.5f);
	
	float4 texColor = float4(1.0f, 1.0f, 1.0f, 1.0f);

	// SM 5.0 Compatible dynamic indexing!
	switch (index) {
		case 0:  texColor = textures[0].Sample(texSampler, tiledUV); break;
		case 1:  texColor = textures[1].Sample(texSampler, tiledUV); break;
		case 2:  texColor = textures[2].Sample(texSampler, tiledUV); break;
		case 3:  texColor = textures[3].Sample(texSampler, tiledUV); break;
		case 4:  texColor = textures[4].Sample(texSampler, tiledUV); break;
		case 5:  texColor = textures[5].Sample(texSampler, tiledUV); break;
		case 6:  texColor = textures[6].Sample(texSampler, tiledUV); break;
		case 7:  texColor = textures[7].Sample(texSampler, tiledUV); break;
		case 8:  texColor = textures[8].Sample(texSampler, tiledUV); break;
		case 9:  texColor = textures[9].Sample(texSampler, tiledUV); break;
		case 10: texColor = textures[10].Sample(texSampler, tiledUV); break;
		case 11: texColor = textures[11].Sample(texSampler, tiledUV); break;
		case 12: texColor = textures[12].Sample(texSampler, tiledUV); break;
		case 13: texColor = textures[13].Sample(texSampler, tiledUV); break;
		case 14: texColor = textures[14].Sample(texSampler, tiledUV); break;
		case 15: texColor = textures[15].Sample(texSampler, tiledUV); break;
	}

	float4 finalColor = texColor * input.color;

	if (finalColor.a < 0.05f) discard;

	PixelOutput output;
	output.color = finalColor;
	output.entityID = -1;

	return output;
}
