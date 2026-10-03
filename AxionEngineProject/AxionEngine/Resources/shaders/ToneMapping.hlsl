struct PS_Input {
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

PS_Input VSMain(uint vertexID : SV_VertexID) {
	PS_Input output;
	output.texCoord = float2((vertexID << 1) & 2, vertexID & 2);
	output.position = float4(output.texCoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
	return output;
}

Texture2D t_hdrImage : register(t0);
SamplerState s_sampler : register(s0);

struct PixelOutput {
	float4 color : SV_TARGET0;
	int entityID : SV_TARGET1;
};

PixelOutput PSMain(PS_Input input) {
	float3 hdrColor = t_hdrImage.Sample(s_sampler, input.texCoord).rgb;
	
	// ACES Tone Mapping
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	float3 mapped = clamp((hdrColor * (a * hdrColor + b)) / (hdrColor * (c * hdrColor + d) + e), 0.0, 1.0);
	
	// Gamma Correction
	mapped = pow(mapped, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
	
	PixelOutput output;
	output.color = float4(mapped, 1.0f);
	output.entityID = -1;
	
	return output;
}
