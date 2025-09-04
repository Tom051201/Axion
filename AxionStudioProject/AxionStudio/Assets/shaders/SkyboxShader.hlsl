cbuffer SceneBuffer : register(b0) {
    float4x4 u_view;
    float4x4 u_projection;
    float4x4 u_viewProjection;
};

struct VSInput {
    float3 pos : POSITION;
};

struct PSInput {
    float4 pos : SV_POSITION;
    float3 dir : TEXCOORD;
};

TextureCube u_skybox : register(t0);
SamplerState u_sampler : register(s0);

PSInput VSMain(VSInput input) {
    PSInput output;

    // Remove translation from view
    float4x4 viewNoTranslation = u_view;
    viewNoTranslation._41 = 0.0f;
    viewNoTranslation._42 = 0.0f;
    viewNoTranslation._43 = 0.0f;

    float4 pos = mul(float4(input.pos, 1.0f), viewNoTranslation);
    output.pos = mul(pos, u_projection);

    output.dir = input.pos;

    return output;
}

float4 PSMain(PSInput input) : SV_TARGET{
    //float3 color = normalize(input.dir) * 0.5f + 0.5f; // remap [-1,1] to [0,1]
    //return float4(color, 1.0f);
    return u_skybox.Sample(u_sampler, normalize(input.dir));
}