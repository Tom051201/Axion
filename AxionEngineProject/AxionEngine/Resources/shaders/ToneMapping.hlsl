cbuffer ToneMapBuffer : register(b1)
{
    int u_toneMappingMode; // 0 = None, 1 = Reinhard, 2 = ACESFilmic, 3 = Uncharted2
    float u_exposure;
    float u_gamma;
    float u_vignetteIntensity;

    float2 u_invRenderSize; // (1.0 / width, 1.0 / height)
    int u_antiAliasingQuality; // 0 = Off, 1 = Low, 2 = Medium, 3 = High, 4 = Ultra
    float u_padding;
};

struct PS_Input
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

PS_Input VSMain(uint vertexID : SV_VertexID)
{
    PS_Input output;
    output.texCoord = float2((vertexID << 1) & 2, vertexID & 2);
    output.position = float4(output.texCoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return output;
}

Texture2D t_hdrImage : register(t0);
SamplerState s_sampler : register(s0);

struct PixelOutput
{
    float4 color : SV_TARGET0;
    int entityID : SV_TARGET1;
};

float3 ACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

float3 Uncharted2Curve(float3 x)
{
    float A = 0.15f;
    float B = 0.50f;
    float C = 0.10f;
    float D = 0.20f;
    float E = 0.02f;
    float F = 0.30f;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float3 Uncharted2Tonemap(float3 color)
{
    float W = 11.2f;
    float3 curr = Uncharted2Curve(color * 2.0f);
    float3 whiteScale = 1.0f / Uncharted2Curve(float3(W, W, W));
    return saturate(curr * whiteScale);
}

// Samples HDR texture and applies exposure + tone mapping into [0, 1] range
float3 SampleTonemapped(float2 uv)
{
    float3 hdrColor = t_hdrImage.SampleLevel(s_sampler, uv, 0).rgb * u_exposure;

    if (u_toneMappingMode == 0)
    {
        return saturate(hdrColor);
    }
    else if (u_toneMappingMode == 1)
    {
        return hdrColor / (hdrColor + float3(1.0f, 1.0f, 1.0f));
    }
    else if (u_toneMappingMode == 2)
    {
        return ACESFilm(hdrColor);
    }
    else
    {
        return Uncharted2Tonemap(hdrColor);
    }
}

// Perceptual Luma (sqrt approximates gamma-space luminance for accurate edge detection in darks)
float RGBToLuma(float3 rgb)
{
    return sqrt(max(dot(rgb, float3(0.299f, 0.587f, 0.114f)), 0.0001f));
}

float3 ApplyFXAA(float2 uv, float3 colorM)
{
    float lumaM = RGBToLuma(colorM);
    float lumaN = RGBToLuma(SampleTonemapped(uv + float2(0.0f, -1.0f) * u_invRenderSize));
    float lumaS = RGBToLuma(SampleTonemapped(uv + float2(0.0f, 1.0f) * u_invRenderSize));
    float lumaW = RGBToLuma(SampleTonemapped(uv + float2(-1.0f, 0.0f) * u_invRenderSize));
    float lumaE = RGBToLuma(SampleTonemapped(uv + float2(1.0f, 0.0f) * u_invRenderSize));

    float lumaMin = min(lumaM, min(min(lumaN, lumaS), min(lumaW, lumaE)));
    float lumaMax = max(lumaM, max(max(lumaN, lumaS), max(lumaW, lumaE)));
    float lumaRange = lumaMax - lumaMin;

	// Quality-tier parameters
    float edgeThreshold = 0.0833f;
    float edgeThresholdMin = 0.0208f;
    float subpixQuality = 0.85f;
    int searchSteps = 8;

    if (u_antiAliasingQuality == 1)
    {
		// LOW
        edgeThreshold = 0.166f;
        edgeThresholdMin = 0.0625f;
        subpixQuality = 0.50f;
        searchSteps = 2;
    }
    else if (u_antiAliasingQuality == 2)
    {
		// MEDIUM
        edgeThreshold = 0.125f;
        edgeThresholdMin = 0.0312f;
        subpixQuality = 0.75f;
        searchSteps = 4;
    }
    else if (u_antiAliasingQuality >= 4)
    {
		// ULTRA
        edgeThreshold = 0.0625f;
        edgeThresholdMin = 0.0156f;
        subpixQuality = 1.00f;
        searchSteps = 12;
    }

	// Early exit on flat areas with no local contrast edge
    if (lumaRange < max(edgeThresholdMin, lumaMax * edgeThreshold))
    {
        return colorM;
    }

	// Sample 4 diagonal corners
    float lumaNW = RGBToLuma(SampleTonemapped(uv + float2(-1.0f, -1.0f) * u_invRenderSize));
    float lumaNE = RGBToLuma(SampleTonemapped(uv + float2(1.0f, -1.0f) * u_invRenderSize));
    float lumaSW = RGBToLuma(SampleTonemapped(uv + float2(-1.0f, 1.0f) * u_invRenderSize));
    float lumaSE = RGBToLuma(SampleTonemapped(uv + float2(1.0f, 1.0f) * u_invRenderSize));

    float lumaNS = lumaN + lumaS;
    float lumaWE = lumaW + lumaE;
    float lumaCorners = lumaNW + lumaNE + lumaSW + lumaSE;

	// Sub-pixel aliasing factor
    float lumaAvg = (2.0f * (lumaNS + lumaWE) + lumaCorners) / 12.0f;
    float subPixelOffset1 = saturate(abs(lumaAvg - lumaM) / lumaRange);
    float subPixelOffset2 = (-2.0f * subPixelOffset1 + 3.0f) * subPixelOffset1 * subPixelOffset1;
    float subPixelFinal = subPixelOffset2 * subPixelOffset2 * subpixQuality;

	// Detect Horizontal vs Vertical Edge Orientation
    float edgeHorz = abs((lumaNW + lumaNE) - 2.0f * lumaN) + 2.0f * abs(lumaWE - 2.0f * lumaM) + abs((lumaSW + lumaSE) - 2.0f * lumaS);
    float edgeVert = abs((lumaNW + lumaSW) - 2.0f * lumaW) + 2.0f * abs(lumaNS - 2.0f * lumaM) + abs((lumaNE + lumaSE) - 2.0f * lumaE);
    bool isHorizontal = (edgeHorz >= edgeVert);

    float luma1 = isHorizontal ? lumaN : lumaW;
    float luma2 = isHorizontal ? lumaS : lumaE;
    float gradient1 = abs(luma1 - lumaM);
    float gradient2 = abs(luma2 - lumaM);

    bool is1Steepest = (gradient1 >= gradient2);
    float gradientScaled = 0.25f * max(gradient1, gradient2);

    float stepLength = isHorizontal ? u_invRenderSize.y : u_invRenderSize.x;
    float lumaLocalAverage = 0.0f;

    if (is1Steepest)
    {
        stepLength = -stepLength;
        lumaLocalAverage = 0.5f * (luma1 + lumaM);
    }
    else
    {
        lumaLocalAverage = 0.5f * (luma2 + lumaM);
    }

	// Shift UV half a pixel perpendicular to the edge
    float2 currentUV = uv;
    if (isHorizontal)
    {
        currentUV.y += stepLength * 0.5f;
    }
    else
    {
        currentUV.x += stepLength * 0.5f;
    }

	// Walk along the edge in both directions to find its endpoints
    float2 offset = isHorizontal ? float2(u_invRenderSize.x, 0.0f) : float2(0.0f, u_invRenderSize.y);
    float2 uv1 = currentUV - offset;
    float2 uv2 = currentUV + offset;

    float lumaEnd1 = RGBToLuma(SampleTonemapped(uv1)) - lumaLocalAverage;
    float lumaEnd2 = RGBToLuma(SampleTonemapped(uv2)) - lumaLocalAverage;
    bool reached1 = (abs(lumaEnd1) >= gradientScaled);
    bool reached2 = (abs(lumaEnd2) >= gradientScaled);

	[loop]
    for (int i = 1; i < searchSteps; i++)
    {
        if (reached1 && reached2)
            break;
        if (!reached1)
        {
            uv1 -= offset;
            lumaEnd1 = RGBToLuma(SampleTonemapped(uv1)) - lumaLocalAverage;
            reached1 = (abs(lumaEnd1) >= gradientScaled);
        }
        if (!reached2)
        {
            uv2 += offset;
            lumaEnd2 = RGBToLuma(SampleTonemapped(uv2)) - lumaLocalAverage;
            reached2 = (abs(lumaEnd2) >= gradientScaled);
        }
    }

    float distance1 = isHorizontal ? (uv.x - uv1.x) : (uv.y - uv1.y);
    float distance2 = isHorizontal ? (uv2.x - uv.x) : (uv2.y - uv.y);

    bool isDirection1 = (distance1 < distance2);
    float distanceFinal = min(distance1, distance2);
    float edgeThickness = max(distance1 + distance2, 0.0001f);

    float pixelOffset = -distanceFinal / edgeThickness + 0.5f;
    bool isLumaCenterSmaller = (lumaM < lumaLocalAverage);
    bool correctVariation = (((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0f) != isLumaCenterSmaller);
    float edgeOffset = correctVariation ? pixelOffset : 0.0f;

	// Combine long-edge offset and sub-pixel stipple offset
    float finalOffset = max(edgeOffset, subPixelFinal);
    float2 finalUV = uv;
    if (isHorizontal)
    {
        finalUV.y += finalOffset * stepLength;
    }
    else
    {
        finalUV.x += finalOffset * stepLength;
    }

    return SampleTonemapped(finalUV);
}

PixelOutput PSMain(PS_Input input)
{
	// 1. Sample center pixel with Exposure & Tone Mapping
    float3 mapped = SampleTonemapped(input.texCoord);

	// 2. Apply FXAA if enabled
    if (u_antiAliasingQuality > 0)
    {
        mapped = ApplyFXAA(input.texCoord, mapped);
    }

	// 3. Apply Vignette
    if (u_vignetteIntensity > 0.001f)
    {
        float2 uv = input.texCoord * (1.0f - input.texCoord.yx);
        float vig = uv.x * uv.y * 15.0f;
        vig = saturate(pow(abs(vig), u_vignetteIntensity));
        mapped *= vig;
    }

	// 4. Gamma Correction
    float invGamma = 1.0f / max(u_gamma, 0.0001f);
    mapped = pow(max(mapped, 0.0f), float3(invGamma, invGamma, invGamma));

    PixelOutput output;
    output.color = float4(mapped, 1.0f);
    output.entityID = -1;

    return output;
}
