cbuffer SceneBuffer : register(b0)
{
	float4x4 u_view;
	float4x4 u_projection;
	float4x4 u_viewProjection;
};

cbuffer ObjectBuffer : register(b1)
{
	float4    u_color;         // tint color OR uniform flat color
	float4x4  u_modelMatrix;
	uint      u_hasTexture;    // 0 = no texture, 1 = sample texture
	float3    _padding;        // align to 16 bytes
};

Texture2D    u_texture : register(t0);
SamplerState u_sampler : register(s0);

struct VSInput
{
	float3 pos     : POSITION;
	float3 normal  : NORMAL;
	float2 uv      : TEXCOORD;
};

struct PSInput
{
	float4 pos       : SV_POSITION;
	float3 worldPos  : POSITION_WS;
	float2 uv        : TEXCOORD;
	float4 color     : COLOR;
};

PSInput VSMain(VSInput input)
{
	PSInput output;

	float4 worldPos = mul(float4(input.pos, 1.0f), u_modelMatrix);
	output.worldPos = worldPos.xyz;
	output.pos = mul(worldPos, u_viewProjection);

	output.uv = input.uv;
	output.color = u_color;

	return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	if (u_hasTexture == 1)
	{
		float4 texColor = u_texture.Sample(u_sampler, input.uv);
		return texColor * input.color; // tint
	}
	else
	{
		return input.color; // pure color quad
	}
}
