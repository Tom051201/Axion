cbuffer CameraBuffer : register(b0) {
	float4x4 u_view;
	float4x4 u_projection;
	float4x4 u_viewProjection;
	float4x4 u_lightSpaceMatrix;
}

struct SkeletalInstanceData {
	float4x4 modelMatrix;
	float4 color;
	uint boneOffset;
	float3 padding;
};

StructuredBuffer<SkeletalInstanceData> u_instanceData : register(t7);
StructuredBuffer<float4x4> u_boneData : register(t8);

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
};

PixelInput VSMain(VertexInput input) {
	PixelInput output;

	SkeletalInstanceData instance = u_instanceData[input.instanceID];
	uint bOffset = instance.boneOffset;
	uint4 bIDs = (uint4)input.boneIDs;

	float4x4 boneTransform = 
		input.boneWeights.x * u_boneData[bOffset + bIDs.x] +
		input.boneWeights.y * u_boneData[bOffset + bIDs.y] +
		input.boneWeights.z * u_boneData[bOffset + bIDs.z] +
		input.boneWeights.w * u_boneData[bOffset + bIDs.w];

	float4 localPos = mul(float4(input.position, 1.0f), boneTransform);
	float4 worldPosition = mul(localPos, instance.modelMatrix);
	
	output.position = mul(worldPosition, u_lightSpaceMatrix);

	return output;
}

void PSMain(PixelInput input) {}