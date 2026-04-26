#include "axpch.h"
#include "EngineAssets.h"

#include "AxionEngine/Resources/shaders/ShadowMap_VS.h"
#include "AxionEngine/Resources/shaders/ShadowMap_PS.h"
#include "AxionEngine/Resources/shaders/Skybox_VS.h"
#include "AxionEngine/Resources/shaders/Skybox_PS.h"
#include "AxionEngine/Resources/shaders/StandardPBR_VS.h"
#include "AxionEngine/Resources/shaders/StandardPBR_PS.h"
#include "AxionEngine/Resources/shaders/SkeletalPBR_VS.h"
#include "AxionEngine/Resources/shaders/SkeletalPBR_PS.h"
#include "AxionEngine/Resources/shaders/SkeletalShadowMap_VS.h"
#include "AxionEngine/Resources/shaders/SkeletalShadowMap_PS.h"

namespace Axion {

	Ref<Texture2D> EngineAssets::s_whiteTexture = nullptr;
	Ref<Texture2D> EngineAssets::s_errorTexture = nullptr;
	Ref<Mesh> EngineAssets::s_cubeMesh = nullptr;
	Ref<Pipeline> EngineAssets::s_skyboxPipeline = nullptr;
	Ref<Pipeline> EngineAssets::s_shadowPipeline = nullptr;
	Ref<Pipeline> EngineAssets::s_standardPBRPipeline = nullptr;
	Ref<Pipeline> EngineAssets::s_skeletalPBRPipeline = nullptr;
	Ref<Pipeline> EngineAssets::s_skeletalShadowPipeline = nullptr;
	Ref<Shader> EngineAssets::s_skyboxShader = nullptr;
	Ref<Shader> EngineAssets::s_shadowShader = nullptr;
	Ref<Shader> EngineAssets::s_standardPBRShader = nullptr;
	Ref<Shader> EngineAssets::s_skeletalPBRShader = nullptr;
	Ref<Shader> EngineAssets::s_skeletalShadowShader = nullptr;

	void EngineAssets::initialize() {
		// -- White Texture --
		uint32_t whiteTextureData = 0xffffffff;
		s_whiteTexture = Texture2D::create(1, 1, &whiteTextureData);

		// -- Error Texture --
		uint32_t errorTextureData[4] = { 0xffff00ff, 0xff000000, 0xff000000, 0xffff00ff };
		s_errorTexture = Texture2D::create(2, 2, &errorTextureData);

		// -- Cube Mesh --
		std::vector<Vertex> vertices = {
			{ -0.5,  0.5, -0.5 },
			{ -0.5, -0.5, -0.5 },
			{  0.5, -0.5, -0.5 },
			{  0.5,  0.5, -0.5 },
			{ -0.5,  0.5,  0.5 },
			{ -0.5, -0.5,  0.5 },
			{  0.5, -0.5,  0.5 },
			{  0.5,  0.5,  0.5 }
		};
		std::vector<uint32_t> indices = {
			// Back face		// Front face
			0, 1, 2,			4, 7, 6,
			2, 3, 0,			6, 5, 4,

			// Left face		// Right face
			4, 5, 1,			3, 2, 6,
			1, 0, 4,			6, 7, 3,

			// Top face			// Bottom face
			4, 0, 3,			1, 5, 6,
			3, 7, 4,			6, 2, 1
		};
		s_cubeMesh = Mesh::create(vertices, indices);

		// -- Shadow Shader --
		ShaderSpecification shadowShaderSpec;
		shadowShaderSpec.name = "ShadowMap";
		shadowShaderSpec.batchTextures = 0;
		s_shadowShader = Shader::create(shadowShaderSpec);
		s_shadowShader->loadFromBytecode(
			g_ShadowMap_VS, sizeof(g_ShadowMap_VS),
			g_ShadowMap_PS, sizeof(g_ShadowMap_PS)
		);

		// -- Shadow Pipeline --
		PipelineSpecification shadowPipeSpec;
		shadowPipeSpec.shader = s_shadowShader;
		shadowPipeSpec.numRenderTargets = 0;
		shadowPipeSpec.colorFormat = ColorFormat::RED_INTEGER;
		shadowPipeSpec.depthStencilFormat = DepthStencilFormat::DEPTH32F;
		shadowPipeSpec.depthTest = true;
		shadowPipeSpec.depthWrite = true;
		shadowPipeSpec.depthFunction = DepthCompare::Less;
		shadowPipeSpec.stencilEnabled = false;
		shadowPipeSpec.sampleCount = 1;
		shadowPipeSpec.cullMode = CullMode::Front;
		shadowPipeSpec.topology = PrimitiveTopology::TriangleList;
		shadowPipeSpec.vertexLayout = {
			{ "POSITION",	ShaderDataType::Float3 },
			{ "NORMAL",		ShaderDataType::Float3 },
			{ "TANGENT",	ShaderDataType::Float3 },
			{ "TEXCOORD",	ShaderDataType::Float2 },
			{ "COLOR",		ShaderDataType::Float4, false, true },
			{ "ROW",		ShaderDataType::Float4, false, true },
			{ "ROW",		ShaderDataType::Float4, false, true },
			{ "ROW",		ShaderDataType::Float4, false, true },
			{ "ROW",		ShaderDataType::Float4, false, true }
		};
		s_shadowPipeline = Pipeline::create(shadowPipeSpec);

		// -- Skybox Shader --
		ShaderSpecification skyboxShaderSpec;
		skyboxShaderSpec.name = "SkyboxShader";
		skyboxShaderSpec.batchTextures = 1;
		s_skyboxShader = Shader::create(skyboxShaderSpec);
		s_skyboxShader->loadFromBytecode(
			g_Skybox_VS, sizeof(g_Skybox_VS),
			g_Skybox_PS, sizeof(g_Skybox_PS)
		);

		// -- Skybox Pipeline --
		PipelineSpecification skyboxPipeSpec;
		skyboxPipeSpec.shader = s_skyboxShader;
		skyboxPipeSpec.numRenderTargets = 1;
		skyboxPipeSpec.colorFormat = ColorFormat::RGBA8;
		skyboxPipeSpec.depthStencilFormat = DepthStencilFormat::DEPTH32F;
		skyboxPipeSpec.depthTest = true;
		skyboxPipeSpec.depthWrite = false;
		skyboxPipeSpec.depthFunction = DepthCompare::LessEqual;
		skyboxPipeSpec.stencilEnabled = false;
		skyboxPipeSpec.sampleCount = 1;
		skyboxPipeSpec.cullMode = CullMode::Back;
		skyboxPipeSpec.topology = PrimitiveTopology::TriangleList;
		skyboxPipeSpec.vertexLayout = {
			{ "POSITION",	ShaderDataType::Float3 }
		};
		s_skyboxPipeline = Pipeline::create(skyboxPipeSpec);

		// -- Standard PBR Shader --
		ShaderSpecification standardPBRShaderSpec;
		standardPBRShaderSpec.name = "StandardPBRShader";
		standardPBRShaderSpec.batchTextures = 16;
		s_standardPBRShader = Shader::create(standardPBRShaderSpec);
		s_standardPBRShader->loadFromBytecode(
			g_StandardPBR_VS, sizeof(g_StandardPBR_VS),
			g_StandardPBR_PS, sizeof(g_StandardPBR_PS)
		);

		// -- Standard PBR Pipeline --
		PipelineSpecification standardPBRPipeSpec;
		standardPBRPipeSpec.shader = s_standardPBRShader;
		standardPBRPipeSpec.numRenderTargets = 1;
		standardPBRPipeSpec.colorFormat = ColorFormat::RGBA8;
		standardPBRPipeSpec.depthStencilFormat = DepthStencilFormat::DEPTH32F;
		standardPBRPipeSpec.depthTest = true;
		standardPBRPipeSpec.depthWrite = true;
		standardPBRPipeSpec.depthFunction = DepthCompare::Less;
		standardPBRPipeSpec.stencilEnabled = false;
		standardPBRPipeSpec.sampleCount = 1;
		standardPBRPipeSpec.cullMode = CullMode::Back;
		standardPBRPipeSpec.topology = PrimitiveTopology::TriangleList;
		standardPBRPipeSpec.vertexLayout = {
			{ "POSITION",	ShaderDataType::Float3 },
			{ "NORMAL",		ShaderDataType::Float3 },
			{ "TANGENT",	ShaderDataType::Float3 },
			{ "TEXCOORD",	ShaderDataType::Float2 },
			{ "COLOR",		ShaderDataType::Float4, false, true },
			{ "ROW",		ShaderDataType::Float4, false, true },
			{ "ROW",		ShaderDataType::Float4, false, true },
			{ "ROW",		ShaderDataType::Float4, false, true },
			{ "ROW",		ShaderDataType::Float4, false, true },
		};
		s_standardPBRPipeline = Pipeline::create(standardPBRPipeSpec);

		// -- Skeletal PBR Shader --
		ShaderSpecification skeletalPBRShaderSpec;
		skeletalPBRShaderSpec.name = "SkeletalPBRShader";
		skeletalPBRShaderSpec.batchTextures = 16;
		s_skeletalPBRShader = Shader::create(skeletalPBRShaderSpec);
		s_skeletalPBRShader->loadFromBytecode(
			g_SkeletalPBR_VS, sizeof(g_SkeletalPBR_VS),
			g_SkeletalPBR_PS, sizeof(g_SkeletalPBR_PS)
		);

		// -- Skeletal PBR Pipeline --
		PipelineSpecification skeletalPBRPipeSpec;
		skeletalPBRPipeSpec.shader = s_skeletalPBRShader;
		skeletalPBRPipeSpec.numRenderTargets = 1;
		skeletalPBRPipeSpec.colorFormat = ColorFormat::RGBA8;
		skeletalPBRPipeSpec.depthStencilFormat = DepthStencilFormat::DEPTH32F;
		skeletalPBRPipeSpec.depthTest = true;
		skeletalPBRPipeSpec.depthWrite = true;
		skeletalPBRPipeSpec.depthFunction = DepthCompare::Less;
		skeletalPBRPipeSpec.stencilEnabled = false;
		skeletalPBRPipeSpec.sampleCount = 1;
		skeletalPBRPipeSpec.cullMode = CullMode::Back;
		skeletalPBRPipeSpec.topology = PrimitiveTopology::TriangleList;
		skeletalPBRPipeSpec.vertexLayout = {
			{ "POSITION",		ShaderDataType::Float3 },
			{ "NORMAL",			ShaderDataType::Float3 },
			{ "TANGENT",		ShaderDataType::Float3 },
			{ "TEXCOORD",		ShaderDataType::Float2 },
			{ "BLENDINDICES",	ShaderDataType::Int4 },
			{ "BLENDWEIGHT",	ShaderDataType::Float4 }
		};
		s_skeletalPBRPipeline = Pipeline::create(skeletalPBRPipeSpec);

		// -- Skeletal Shadow Shader --
		ShaderSpecification skelShadowShaderSpec;
		skelShadowShaderSpec.name = "SkeletalShadowMap";
		skelShadowShaderSpec.batchTextures = 0;
		s_skeletalShadowShader = Shader::create(skelShadowShaderSpec);
		s_skeletalShadowShader->loadFromBytecode(
			g_SkeletalShadowMap_VS, sizeof(g_SkeletalShadowMap_VS),
			g_SkeletalShadowMap_PS, sizeof(g_SkeletalShadowMap_PS)
		);

		// -- Skeletal Shadow Pipeline --
		PipelineSpecification skelShadowPipeSpec;
		skelShadowPipeSpec.shader = s_skeletalShadowShader;
		skelShadowPipeSpec.numRenderTargets = 0;
		skelShadowPipeSpec.colorFormat = ColorFormat::RED_INTEGER;
		skelShadowPipeSpec.depthStencilFormat = DepthStencilFormat::DEPTH32F;
		skelShadowPipeSpec.depthTest = true;
		skelShadowPipeSpec.depthWrite = true;
		skelShadowPipeSpec.depthFunction = DepthCompare::Less;
		skelShadowPipeSpec.stencilEnabled = false;
		skelShadowPipeSpec.sampleCount = 1;
		skelShadowPipeSpec.cullMode = CullMode::Front;
		skelShadowPipeSpec.topology = PrimitiveTopology::TriangleList;
		skelShadowPipeSpec.vertexLayout = {
			{ "POSITION",		ShaderDataType::Float3 },
			{ "NORMAL",			ShaderDataType::Float3 },
			{ "TANGENT",		ShaderDataType::Float3 },
			{ "TEXCOORD",		ShaderDataType::Float2 },
			{ "BLENDINDICES",	ShaderDataType::Int4 },
			{ "BLENDWEIGHT",	ShaderDataType::Float4 }
		};
		s_skeletalShadowPipeline = Pipeline::create(skelShadowPipeSpec);

		AX_CORE_LOG_INFO("Engine Assets initialized");
	}

	void EngineAssets::shutdown() {
		s_whiteTexture->release();
		s_errorTexture->release();

		s_cubeMesh->release();

		s_skyboxShader->release();
		s_shadowShader->release();
		s_standardPBRShader->release();
		s_skeletalPBRShader->release();
		s_skeletalShadowShader->release();

		s_skyboxPipeline->release();
		s_shadowPipeline->release();
		s_standardPBRPipeline->release();
		s_skeletalPBRPipeline->release();
		s_skeletalShadowPipeline->release();

		AX_CORE_LOG_INFO("Engine Assets shutdown");
	}

}
