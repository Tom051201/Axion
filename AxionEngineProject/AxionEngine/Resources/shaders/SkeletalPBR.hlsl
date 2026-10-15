Texture2D g_Textures[] : register(t0, space0);
TextureCube g_CubeTextures[] : register(t0, space2);
SamplerState s_sampler : register(s0);

struct DirectionalLight {
	float4 direction;
	float4 color;
};

struct PointLight {
	float4 position;
	float4 color;
	float4 params; // x = radius, y = falloff, z = padding, w = padding
};

struct SpotLight {
	float4 position;
	float4 direction;
	float4 color;
	float4 params; // x = range, y = inner cutoff, z = outer cutoff, w = padding
};

cbuffer CameraBuffer : register(b0)
{
	float4x4 u_viewProjection;
	float4x4 u_skyboxViewProjection;
	float4x4 u_lightSpaceMatrix;

	float4 u_cameraPosition;
	float4 u_ambientColor;

	int u_hasDirectionalLight;
	int u_pointLightsCount;
	int u_spotLightsCount;
	int u_shadowMapIndex;

	int u_skyboxIndex;
	int3 padding1;

	DirectionalLight u_directionalLight;
	PointLight u_pointLights[16];
	SpotLight u_spotLights[16];
}

cbuffer MaterialBuffer : register(b1) {
	float4 u_albedoColor;
	float u_metalness;
	float u_roughness;
	float u_emissionStrength;
	float u_tiling;
	float u_useNormalMap;
	float u_useMetalnessMap;
	float u_useRoughnessMap;
	float u_useOcclusionMap;
	float u_useEmissiveMap;
	float3 padding;
}

struct SkeletalInstanceData {
	float4x4 modelMatrix;
	float4 color;
	uint boneOffset;
	int entityID;
	int albedoIndex;
	int normalIndex;
	int roughnessIndex;
	int metalnessIndex;
	int occlusionIndex;
	int emissiveIndex;
	int2 padding;
};

StructuredBuffer<SkeletalInstanceData> u_instanceData : register(t0, space1);
StructuredBuffer<float4x4> u_boneData : register(t1, space1);

static const float PI = 3.14159265359;

struct VertexInput {
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 texCoord : TEXCOORD;
	int4 boneIDs : BLENDINDICES;
	float4 boneWeights : BLENDWEIGHT;
	uint instanceID : SV_InstanceID;
};

struct PixelInput {
	float4 position : SV_POSITION;
	float3 worldPos : POSITIONT;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float3x3 tbn : TANGENT;
	float4 color : COLOR;
	nointerpolation int entityID : ENTITY_ID;
	nointerpolation int4 texIndices : TEX_INDICES;
	nointerpolation int4 texIndicesExt : TEX_INDICES_EXT;
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

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness) {
	return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Epic Games' analytical approximation of the BRDF LUT (Karis 2014)
float2 EnvBRDFApprox(float roughness, float NoV) {
	const float4 c0 = float4(-1.0, -0.0275, -0.572, 0.022);
	const float4 c1 = float4(1.0, 0.0425, 1.04, -0.04);
	float4 r = roughness * c0 + c1;
	float a004 = min(r.x * r.x, exp2(-9.28 * NoV)) * r.x + r.y;
	float2 AB = float2(-1.04, 1.04) * a004 + r.zw;
	return AB;
}

float3 CalculateLight(float3 N, float3 V, float3 L, float3 radiance, float3 albedo, float roughness, float metalness) {
	float3 H = normalize(V + L);

	float3 F0 = float3(0.04, 0.04, 0.04);
	F0 = lerp(F0, albedo, metalness);

	float NDF = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);
	float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

	float3 numerator = NDF * G * F;
	float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
	float3 specular = numerator / denom;

	float3 kS = F;
	float3 kD = float3(1.0, 1.0, 1.0) - kS;
	kD *= 1.0 - metalness;

	float NdotL = max(dot(N, L), 0.0);

	return (kD * albedo / PI + specular) * radiance * NdotL;
}

float3 getNormalFromMap(PixelInput input, float2 uv) {
	float3 tangentNormal = g_Textures[NonUniformResourceIndex(input.texIndices.y)].Sample(s_sampler, uv).xyz * 2.0 - 1.0;
	return normalize(mul(tangentNormal, input.tbn));
}

struct PixelOutput {
	float4 color : SV_TARGET0;
	int entityID : SV_TARGET1;
};

PixelInput VSMain(VertexInput input) {
	PixelInput output;

	SkeletalInstanceData instance = u_instanceData[input.instanceID];
	uint bOffset = instance.boneOffset;
	uint4 bIDs = (uint4) input.boneIDs;

	float4x4 boneTransform =
		input.boneWeights.x * u_boneData[bOffset + bIDs.x] +
		input.boneWeights.y * u_boneData[bOffset + bIDs.y] +
		input.boneWeights.z * u_boneData[bOffset + bIDs.z] +
		input.boneWeights.w * u_boneData[bOffset + bIDs.w];

	float4 localPos = mul(float4(input.position, 1.0f), boneTransform);
	float4 worldPosition = mul(localPos, instance.modelMatrix);

	output.worldPos = worldPosition.xyz;
	output.position = mul(worldPosition, u_viewProjection);
	output.texCoord = input.texCoord;
	output.color = instance.color;

	float3x3 boneRot = (float3x3) boneTransform;
	float3x3 modelRot = (float3x3) instance.modelMatrix;
	
	float3 localTangent = mul(input.tangent, boneRot);
	float3 localNormal = mul(input.normal, boneRot);

	float3 T = normalize(mul(localTangent, modelRot));
	float3 N = normalize(mul(localNormal, modelRot));

	T = normalize(T - dot(T, N) * N);
	float3 B = cross(N, T);

	output.normal = N;
	output.tbn = float3x3(T, B, N);
	output.entityID = instance.entityID;
	output.texIndices = int4(instance.albedoIndex, instance.normalIndex, instance.roughnessIndex, instance.metalnessIndex);
	output.texIndicesExt = int4(instance.occlusionIndex, instance.emissiveIndex, 0, 0);

	return output;
}

PixelOutput PSMain(PixelInput input) {
	float2 uv = input.texCoord * max(u_tiling, 1.0f);

	float4 albedoSample = g_Textures[NonUniformResourceIndex(input.texIndices.x)].Sample(s_sampler, uv);
	float3 albedo = pow(max(albedoSample.rgb, 0.0), 2.2) * input.color.rgb * u_albedoColor.rgb;

	// -- Normal mapping --
	float3 N = normalize(input.normal);
	if (u_useNormalMap > 0.5) {
		N = getNormalFromMap(input, uv);
	}

	// -- PBR Maps --
	float metalness = u_metalness;
	if (u_useMetalnessMap > 0.5)
	{
		metalness *= g_Textures[NonUniformResourceIndex(input.texIndices.w)].Sample(s_sampler, uv).r;
	}

	float roughness = u_roughness;
	if (u_useRoughnessMap > 0.5)
	{
		roughness *= g_Textures[NonUniformResourceIndex(input.texIndices.z)].Sample(s_sampler, uv).r;
	}
	roughness = max(roughness, 0.05);

	float ao = 1.0f;
	if (u_useOcclusionMap > 0.5)
	{
		ao = g_Textures[NonUniformResourceIndex(input.texIndicesExt.x)].Sample(s_sampler, uv).r;
	}
	
	float3 emissive = float3(0, 0, 0);
	if (u_useEmissiveMap > 0.5)
	{
		emissive = g_Textures[NonUniformResourceIndex(input.texIndicesExt.y)].Sample(s_sampler, uv).rgb * u_emissionStrength;
	}
	
	// -- Lighting setup --
	float3 camPos = u_cameraPosition.xyz;
	float3 V = normalize(camPos - input.worldPos);
	float3 Lo = float3(0.0, 0.0, 0.0);

	// -- Shadow Mapping calculation --
	float shadow = 0.0f;
	float4 fragPosLightSpace = mul(float4(input.worldPos, 1.0), u_lightSpaceMatrix);

	float3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords.x = projCoords.x * 0.5 + 0.5;
	projCoords.y = -projCoords.y * 0.5 + 0.5;

	if (projCoords.z <= 1.0 && projCoords.x >= 0.0 && projCoords.x <= 1.0 && projCoords.y >= 0.0 && projCoords.y <= 1.0) {
		float currentDepth = projCoords.z;
		float3 L_dir = normalize(u_directionalLight.direction.xyz);
//		float bias = max(0.001 * (1.0 - dot(N, L_dir)), 0.0001);
		float bias = max(0.0002 * (1.0 - dot(N, L_dir)), 0.00005);

		float shadowSum = 0.0;
		float2 texelSize = 1.0 / 2048.0;

		for (int x = -1; x <= 1; ++x) {
			for (int y = -1; y <= 1; ++y) {
				float pcfDepth = g_Textures[NonUniformResourceIndex(u_shadowMapIndex)].SampleLevel(s_sampler, projCoords.xy + float2(x, y) * texelSize, 0).r;
				shadowSum += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
			}
		}
		shadow = shadowSum / 9.0;
	}

	// -- Directional light --
	if (u_hasDirectionalLight > 0) {
		float3 L_dir = normalize(u_directionalLight.direction.xyz);
		float3 radiance_dir = u_directionalLight.color.rgb * 2.0;
		float3 lightCalc = CalculateLight(N, V, L_dir, radiance_dir, albedo, roughness, metalness);

		// -- Shadow Mapping calculation --
		float shadow = 0.0f;
		float4 fragPosLightSpace = mul(float4(input.worldPos, 1.0), u_lightSpaceMatrix);

		float3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
		projCoords.x = projCoords.x * 0.5 + 0.5;
		projCoords.y = -projCoords.y * 0.5 + 0.5;

		if (projCoords.z <= 1.0 && projCoords.x >= 0.0 && projCoords.x <= 1.0 && projCoords.y >= 0.0 && projCoords.y <= 1.0) {
			float currentDepth = projCoords.z;
			float bias = max(0.0002 * (1.0 - dot(N, L_dir)), 0.00005);

			float shadowSum = 0.0;
			float2 texelSize = 1.0 / 2048.0;

			for (int x = -1; x <= 1; ++x) {
				for (int y = -1; y <= 1; ++y) {
					float pcfDepth = g_Textures[NonUniformResourceIndex(u_shadowMapIndex)].SampleLevel(s_sampler, projCoords.xy + float2(x, y) * texelSize, 0).r;
					shadowSum += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
				}
			}
			shadow = shadowSum / 9.0;
		}

		Lo += lightCalc * (1.0 - shadow);
	}

	// -- Point light --
	for (int j = 0; j < u_pointLightsCount; ++j) {
		float3 lightVec = u_pointLights[j].position.xyz - input.worldPos;
		float distance = length(lightVec);

		float radius = u_pointLights[j].params.x;
		float falloffExponent = u_pointLights[j].params.y;

		if (distance < radius) {
			float3 L_point = lightVec / distance;
			float invSquare = 1.0 / (distance * distance + 0.0001);
			float distanceRatio = distance / radius;
			float window = saturate(1.0 - pow(abs(distanceRatio), falloffExponent));

			float attenuation = invSquare * (window * window);
			float3 radiance_point = u_pointLights[j].color.rgb * attenuation;

			Lo += CalculateLight(N, V, L_point, radiance_point, albedo, roughness, metalness);
		}
	}

	// -- Spot light --
	for (int k = 0; k < u_spotLightsCount; ++k) {
		float3 spotLightVec = u_spotLights[k].position.xyz - input.worldPos;
		float spotDistance = length(spotLightVec);
		float spotRange = u_spotLights[k].params.x;

		if (spotDistance < spotRange) {
			float3 L_spot = spotLightVec / spotDistance;

			float invSquare = 1.0 / (spotDistance * spotDistance + 0.0001);
			float distanceRatio = spotDistance / spotRange;
			float window = saturate(1.0 - distanceRatio);
			float distanceAttenuation = invSquare * (window * window);

			float theta = dot(-L_spot, normalize(u_spotLights[k].direction.xyz));

			float innerCutoff = u_spotLights[k].params.y;
			float outerCutoff = u_spotLights[k].params.z;
			float epsilon = innerCutoff - outerCutoff;

			float spotIntensity = saturate((theta - outerCutoff) / epsilon);

			float3 radiance_spot = u_spotLights[k].color.rgb * distanceAttenuation * spotIntensity;

			if (spotIntensity > 0.0) {
				Lo += CalculateLight(N, V, L_spot, radiance_spot, albedo, roughness, metalness);
			}
		}
	}

	// -- Image Based Lighting (IBL Ambient) --
	float3 ambient = float3(0, 0, 0);

	if (u_skyboxIndex >= 0) {
		float3 irradiance = g_CubeTextures[NonUniformResourceIndex(u_skyboxIndex)].SampleLevel(s_sampler, N, 8.0).rgb;
		float3 diffuseIBL = irradiance * albedo;

		float3 R = reflect(-V, N);
		float MAX_REFLECTION_LOD = 8.0;
		float3 prefilteredColor = g_CubeTextures[NonUniformResourceIndex(u_skyboxIndex)].SampleLevel(s_sampler, R, roughness * MAX_REFLECTION_LOD).rgb;

		float2 envBRDF = EnvBRDFApprox(roughness, max(dot(N, V), 0.0));
		
		float3 F0 = float3(0.04, 0.04, 0.04);
		F0 = lerp(F0, albedo, metalness);
		float3 F_IBL = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

		float3 specularIBL = prefilteredColor * (F_IBL * envBRDF.x + envBRDF.y);

		float3 kS_IBL = F_IBL;
		float3 kD_IBL = 1.0 - kS_IBL;
		kD_IBL *= 1.0 - metalness;

		float iblIntensity = u_ambientColor.a;
		ambient = (kD_IBL * diffuseIBL + specularIBL) * ao * iblIntensity;
	}
	else {
		ambient = u_ambientColor.rgb * albedo * ao;
	}

	float3 color = ambient + Lo + emissive;

	PixelOutput output;
	output.color = float4(color, 1.0);
	output.entityID = input.entityID;
	return output;
}
