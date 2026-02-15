cbuffer CameraBuffer : register(b0) {
	float4x4 u_viewProjection;
	float3 u_cameraPosition;
	float u_pad0;
}

cbuffer ObjectBuffer : register(b1) {
	float4x4 u_transform;
}

cbuffer MaterialBuffer : register(b0) {
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

PixelInput VSMain(VertexInput input) {
	PixelInput output;

	float worldPosition = mul(float4(input.position, 1.0f), u_transform);
	output.position = mul(worldPosition, u_viewProjection);
	output.worldPos = worldPosition.xyz;
	output.texCoord = input.texCoord * u_tiling;

	float3 t = normalize(mul(input.tangent, (float3x3)u_transform));
	float3 n = normalize(mul(input.normal, (float3x3)u_transform));
	t = normalize(t - dot(t, n) * n);
	float3 b = cross(n, t);
	output.tbn = float3x3(t, b, n);

	output.normal = n;

	return output;
}

float3 getNormalFromMap(PixelInput input) {
	float3 tangentNormal = t_normal.Sample(s_sampler, input.texCoord).xyz * 2.0 - 1.0;
	return normalize(mul(tangentNormal, input.tbn));
}

float4 PSMain(PixelInput) : SV_TARGET{
	float4 albedoSample = t_albedo.Sample(s_sampler, input.texCoord);
	float3 albedo = albedoSample.rgb * u_albedoColor.rgb;

	float3 n = input.normal;
	if (u_useNormalMap > 0.5) n = getNormalMap(input);

	float metalness = u_metalness;
	if (u_useMetalnessMap > 0.5) metalness *= t_metalness.Sample(s_sampler, input.texCoord).r;

	float roughness = u_Roughness;
	if (u_useRoughnessMap > 0.5) roughness *= t_roughness.Sample(s_sampler, input.texCoord).r;

	float ao = 1.0f;
	if (u_useOcclusionMap > 0.5) ao = t_occlusion.Sample(s_sampler, input.texCoord).r;

	// Lighting
	float3 lightDir = normalize(float3(0.5, 1.0, -0.5));
	float3 viewDir = normalize(u_cameraPosition - input.worldPos);
	float3 h = normalize(lightDir + viewDir);

	// Ambient
	float3 ambient = 0.03 * albedo * ao;

	// Diffuse
	float nDotL = max(dot(n, lightDir), 0.0);
	float3 diffuse = albedo * nDotL;

	// Specular
	float nDotH = max(dot(n, h), 0.0);
	float specular = pow(nDotH, (1.0 - roughness) * 128.0) * metalness;

	// Emission
	// TODO

	float3 finalColor = ambient + diffuse + specular;

	return float4(finalColor, 1.0f);
}
