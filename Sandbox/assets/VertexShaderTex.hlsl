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

cbuffer Camera : register(b0) {
    float4x4 camera;
}

cbuffer Transform : register(b1)
{
    float4x4 modelViewProj;
};

PSInput main(VSInput input) {
    PSInput output;
    output.position = mul(modelViewProj, float4(input.position, 1.0f));
    output.color = input.color;
    output.texCoord = input.texCoord;
    return output;
}
