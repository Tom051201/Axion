// Input structure from vertex buffer
struct VSInput {
    float3 position : POSITION;
    float4 color    : COLOR;
    float2 texCoord : TEXCOORD;
};

// Output to the pixel shader
struct PSInput {
    float4 position : SV_POSITION;
    float4 color    : COLOR;
    float2 texCoord : TEXCOORD;
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 main(PSInput input) : SV_TARGET{
    float4 texColor = tex.Sample(samp, input.texCoord);
    return texColor * input.color; // Modulate texture with vertex color
}