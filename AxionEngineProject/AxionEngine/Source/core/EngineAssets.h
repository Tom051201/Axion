#pragma once

#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Texture.h"
#include "AxionEngine/Source/render/Pipeline.h"

namespace Axion {

	class EngineAssets {
	public:

		static void initialize();
		static void shutdown();

		// ----- Textures -----
		static Ref<Texture2D> getWhiteTexture() { return s_whiteTexture; }
		static Ref<Texture2D> getErrorTexture() { return s_errorTexture; }

		// ----- Meshes -----
		static Ref<Mesh> getCubeMesh() { return s_cubeMesh; }

		// ----- Pipelines -----
		static Ref<Pipeline> getSkyboxPipeline() { return s_skyboxPipeline; }
		static Ref<Pipeline> getShadowPipeline() { return s_shadowPipeline; }
		static Ref<Pipeline> getStandardPBRPipeline() { return s_standardPBRPipeline; }
		static Ref<Pipeline> getSkeletalPBRPipeline() { return s_skeletalPBRPipeline; }
		static Ref<Pipeline> getSkeletalShadowPipeline() { return s_skeletalShadowPipeline; }

		// ----- Shaders -----
		static Ref<Shader> getSkyboxShader() { return s_skyboxShader; }
		static Ref<Shader> getShadowShader() { return s_shadowShader; }
		static Ref<Shader> getStandardPBRShader() { return s_standardPBRShader; }
		static Ref<Shader> getSkeletalPBRShader() { return s_skeletalPBRShader; }
		static Ref<Shader> getSkeletalShadowShader() { return s_skeletalShadowShader; }

	private:

		static Ref<Texture2D> s_whiteTexture;
		static Ref<Texture2D> s_errorTexture;

		static Ref<Mesh> s_cubeMesh;

		static Ref<Pipeline> s_skyboxPipeline;
		static Ref<Pipeline> s_shadowPipeline;
		static Ref<Pipeline> s_standardPBRPipeline;
		static Ref<Pipeline> s_skeletalPBRPipeline;
		static Ref<Pipeline> s_skeletalShadowPipeline;

		static Ref<Shader> s_skyboxShader;
		static Ref<Shader> s_shadowShader;
		static Ref<Shader> s_standardPBRShader;
		static Ref<Shader> s_skeletalPBRShader;
		static Ref<Shader> s_skeletalShadowShader;

	};

}
