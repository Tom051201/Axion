Texture2D<float4> g_Textures[] : register(t0, space0);
RWTexture2D<float4> g_RWTextures[] : register(u0, space0);
SamplerState s_sampler : register(s0);

cbuffer BloomData : register(b1)
{
    int u_srcIndex;
    int u_dstIndex;
    int u_mode;
    float u_threshold;
    float2 u_texelSize;
    float u_blurRadius;
    float u_padding;
}

[numthreads(8, 8, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    // Calculate the normalized UV coordinates for this thread's pixel center
    float2 uv = (float2(DTid.xy) + 0.5f) * u_texelSize;
    float4 color = float4(0, 0, 0, 1);

    if (u_mode == 0)
    {
        // --- PASS 1: EXTRACT ---
        // Use Linear Sampling to smoothly downsample from Full-Res to Half-Res
        color = g_Textures[NonUniformResourceIndex(u_srcIndex)].SampleLevel(s_sampler, uv, 0);
        
        // Smooth Threshold (Prevents harsh jagged edges on bright objects)
        float brightness = max(color.r, max(color.g, color.b));
        float contribution = max(0.0, brightness - u_threshold);
        contribution /= max(brightness, 0.00001); // Prevent divide by zero
        
        g_RWTextures[NonUniformResourceIndex(u_dstIndex)][DTid.xy] = float4(color.rgb * contribution, 1.0);
    }
    else if (u_mode == 1 || u_mode == 2)
    {
        // --- PASS 2 & 3: GAUSSIAN BLUR ---
        float weights[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };
        
        color = g_Textures[NonUniformResourceIndex(u_srcIndex)].SampleLevel(s_sampler, uv, 0) * weights[0];
        
        for (int i = 1; i < 5; ++i)
        {
            // Because we use UVs, the radius smoothly interpolates between pixels!
            float2 offset = (u_mode == 1) ? float2(i * u_blurRadius * u_texelSize.x, 0) : float2(0, i * u_blurRadius * u_texelSize.y);
            
            color += g_Textures[NonUniformResourceIndex(u_srcIndex)].SampleLevel(s_sampler, uv + offset, 0) * weights[i];
            color += g_Textures[NonUniformResourceIndex(u_srcIndex)].SampleLevel(s_sampler, uv - offset, 0) * weights[i];
        }
        g_RWTextures[NonUniformResourceIndex(u_dstIndex)][DTid.xy] = color;
    }
    else if (u_mode == 3)
    {
        // --- PASS 4: COMPOSITE ---
        float4 scene = g_RWTextures[NonUniformResourceIndex(u_dstIndex)][DTid.xy];
        
        // Bilinear upsampling! Smooths out the half-res blockiness perfectly.
        float4 bloom = g_Textures[NonUniformResourceIndex(u_srcIndex)].SampleLevel(s_sampler, uv, 0);
        
        g_RWTextures[NonUniformResourceIndex(u_dstIndex)][DTid.xy] = scene + bloom;
    }
}
