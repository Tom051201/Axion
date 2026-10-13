TextureCube g_CubeTextures[] : register(t0, space0);
SamplerState u_sampler : register(s0);

cbuffer SceneBuffer : register(b0) {
    float4x4 u_viewProjection;
    float4x4 u_skyboxViewProjection;
};

cbuffer SkyboxBuffer : register(b1) {
    int u_skyboxIndex;
    float3 padding;
};

struct VSInput {
    float3 pos : POSITION;
};

struct PSInput {
	float4 pos : SV_POSITION;
	float3 dir : TEXCOORD;
};

struct PixelOutput {
	float4 color : SV_TARGET0;
	int entityID : SV_TARGET1;
};

PSInput VSMain(VSInput input) {
    PSInput output;

    output.pos = mul(float4(input.pos, 1.0f), u_skyboxViewProjection);
    output.pos.z = output.pos.w;
    output.dir = input.pos;

    return output;
}

PixelOutput PSMain(PSInput input) {
    float4 texColor = g_CubeTextures[NonUniformResourceIndex(u_skyboxIndex)].Sample(u_sampler, normalize(input.dir));
    float3 linearColor = pow(max(texColor.rgb, 0.0), 2.2);

    PixelOutput output;
    output.color = float4(linearColor, texColor.a);
    output.entityID = -1;
    
    return output;
}
