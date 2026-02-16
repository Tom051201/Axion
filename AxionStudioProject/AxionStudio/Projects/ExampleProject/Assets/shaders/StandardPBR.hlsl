cbuffer CameraBuffer : register(b0) {
	float4x4 u_view;
	float4x4 u_projection;
	float4x4 u_viewProjection;
}

cbuffer ObjectBuffer : register(b1) {
	float4 u_color;
	float4x4 u_transform;
}

cbuffer MaterialBuffer : register(b2) {
	float4 u_albedoColor;

	float u_metalness;
	float u_roughness;
	float u_emissionStrength;
	float u_tiling;

	float u_useNormalMap;
	float u_useMetalnessMap;
	float u_useRoughnessMap;
	float u_useOcclusionMap;
}

Texture2D t_albedo : register(t0);
Texture2D t_normal : register(t1);
Texture2D t_metalness : register(t2);
Texture2D t_roughness : register(t3);
Texture2D t_occlusion : register(t4);
Texture2D t_emissive : register(t5);

SamplerState s_sampler : register(s0);

static const float PI = 3.14159265359;

struct VertexInput {
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 texCoord : TEXCOORD;
};

struct PixelInput {
	float4 position : SV_POSITION;
	float3 worldPos : POSITIONT;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float3x3 tbn : TANGENT;
};

// Distribution: How many microfacets are aligned with the light?
float DistributionGGX(float3 N, float3 H, float roughness) {
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / max(denom, 0.0000001);
}

// Geometry: Self-shadowing of the surface bumps
float GeometrySchlickGGX(float NdotV, float roughness) {
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness) {
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

// Fresnel: Reflectivity at different viewing angles
float3 FresnelSchlick(float cosTheta, float3 F0) {
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Camera helper
float3 getCameraPosition() {
	float3x3 rot = transpose((float3x3)u_view);
	float3 trans = float3(u_view._41, u_view._42, u_view._43);
	return mul(-trans, rot);
}

float3 getNormalFromMap(PixelInput input, float2 uv) {
	float3 tangentNormal = t_normal.Sample(s_sampler, uv).xyz * 2.0 - 1.0;
	return normalize(mul(tangentNormal, input.tbn));
}

PixelInput VSMain(VertexInput input) {
	PixelInput output;

	float4 worldPosition = mul(float4(input.position, 1.0f), u_transform);
	output.worldPos = worldPosition.xyz;
	output.position = mul(worldPosition, u_viewProjection);
	output.texCoord = input.texCoord;

	// Calculate TBN Matrix
	float3x3 normalMatrix = (float3x3)u_transform;
	float3 T = normalize(mul(input.tangent, normalMatrix));
	float3 N = normalize(mul(input.normal, normalMatrix));

	// Gram-Schmidt Orthogonalization
	T = normalize(T - dot(T, N) * N);
	float3 B = cross(N, T);

	output.normal = N;
	output.tbn = float3x3(T, B, N);

	return output;
}

float4 PSMain(PixelInput input) : SV_TARGET{
	float2 uv = input.texCoord * max(u_tiling, 1.0f);

	float4 albedoSample = t_albedo.Sample(s_sampler, uv);
	float3 albedo = pow(albedoSample.rgb, 2.2) * u_albedoColor.rgb;

	// -- Normal mapping --
	float3 N = normalize(input.normal);
	if (u_useNormalMap > 0.5) N = getNormalFromMap(input, uv);

	// -- PBR Maps --
	float metalness = u_metalness;
	if (u_useMetalnessMap > 0.5) metalness *= t_metalness.Sample(s_sampler, uv).r;

	float roughness = u_roughness;
	if (u_useRoughnessMap > 0.5) roughness *= t_roughness.Sample(s_sampler, uv).r;
	roughness = max(roughness, 0.05);

	float ao = 1.0f;
	if (u_useOcclusionMap > 0.5) ao = t_occlusion.Sample(s_sampler, uv).r;

	// -- Lighting setup --
	float3 camPos = getCameraPosition();
	float3 V = normalize(camPos - input.worldPos);
	float3 lightDir = normalize(float3(0.5, 1.0, -0.5));
	float3 L = normalize(lightDir);
	float3 H = normalize(V + L);
	float3 radiance = float3(1.0, 1.0, 1.0) * 2.0;

	// -- Base reflectivity : 0.04 for Dielectrics (Plastic), Albedo color for Metals --
	float3 F0 = float3(0.04, 0.04, 0.04);
	F0 = lerp(F0, albedo, metalness);

	// -- Cook-Torrance BRDF --
	float NDF = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);
	float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

	float3 numerator = NDF * G * F;
	float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
	float3 specular = numerator / denom;

	// -- Energy conservation --
	float3 kS = F;
	float3 kD = float3(1.0, 1.0, 1.0) - kS;
	kD *= 1.0 - metalness;

	float NdotL = max(dot(N, L), 0.0);

	// -- Final radiance --
	float3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;

	// -- Ambient --
	float3 ambient = float3(0.03, 0.03, 0.03) * albedo * ao;
	float3 color = ambient + Lo;

	// -- Post Processing --
	// HDR Tone Mapping
	color = color / (color + float3(1.0, 1.0, 1.0));
	// Gamma Correction (Linear -> sRGB)
	color = pow(color, 1.0 / 2.2);

	// DEBUG
	//float3 T = input.tbn[0];
	//return float4(T * 0.5 + 0.5, 1.0);

	return float4(color, 1.0);
}
