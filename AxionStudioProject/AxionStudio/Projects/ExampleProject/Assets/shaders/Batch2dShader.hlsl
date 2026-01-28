cbuffer CameraData : register(b0) {
	float4x4 ViewProjection;
};

struct VertexInput {
	float3 Position : POSITION;
	float4 Color : COLOR;
	float2 TexCoord : TEXCOORD;
	float TexIndex : TEXINDEX;
	float Tiling : TILING;
};

struct PixelInput {
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
	float2 TexCoord : TEXCOORD;
	float TexIndex : TEXINDEX; // Pass to PS for later use
};

PixelInput VSMain(VertexInput input) {
	PixelInput output;
	// Input position is already in World Space calculated by CPU
	output.Position = mul(float4(input.Position, 1.0f), ViewProjection);
	output.Color = input.Color;
	output.TexCoord = input.TexCoord;
	output.TexIndex = input.TexIndex;
	return output;
}

float4 PSMain(PixelInput input) : SV_TARGET {
	return input.Color;
}
