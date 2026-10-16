#pragma once

#include <cstdint>
#include <vector>
#include <functional>
#include <array>

#include "AxionEngine/Source/core/Window.h"
#include "AxionEngine/Source/core/Timer.h"
#include "AxionEngine/Source/graphics/Camera.h"
#include "AxionEngine/Source/graphics/Mesh.h"
#include "AxionEngine/Source/graphics/Shader.h"
#include "AxionEngine/Source/graphics/Texture.h"
#include "AxionEngine/Source/graphics/FrameBuffer.h"

namespace Axion {

	class RenderContext;

	enum class RendererAPI {
		None = 0,
		DirectX12 = 1,
		DirectX11 = 2
	};

	constexpr uint32_t MAX_POINT_LIGHTS = 16;
	constexpr uint32_t MAX_SPOT_LIGHTS = 16;

	struct DirectionalLightData {
		Vec3 direction;
		Vec4 color;
	};

	struct PointLightData {
		Vec3 position;
		Vec4 color;
		float radius;
		float falloff;
	};

	struct SpotLightData {
		Vec3 position;
		Vec3 direction;
		Vec4 color;
		float range;
		float innerCutoff;
		float outerCutoff;
	};

	struct LightingData {
		Vec4 ambientColor;

		DirectionalLightData directionalLight;
		bool hasDirectionalLight = false;
		std::vector<PointLightData> pointLights;
		std::vector<SpotLightData> spotLights;
		int skyboxIndex = -1;
	};

	enum class RootSlot : uint32_t {
		Camera = 0,			// DX12: b0, space0
		Material = 1,		// DX12: b1, space0
		InstanceData = 2,	// DX12: t0, space1
		BoneData = 3,		// DX12: t1, space1
		Textures = 4,		// DX12: t0, space0
		TextureCubes = 5,	// DX12: t0, space2
		UAVs = 6,			// DX12: u0, space0
	};

	struct RendererStats {
		uint32_t drawCalls = 0;
		uint32_t quadCount2D = 0;
		uint32_t lineCount2D = 0;
		uint32_t meshCount3D = 0;
		uint32_t instanceCount3D = 0;

		uint32_t getTotalVertexCount2D() const { return quadCount2D * 4 + lineCount2D * 2; }
		uint32_t getTotalIndexCount2D() const { return quadCount2D * 6; }
	};

	enum class GraphicsQuality {
		Off = 0,
		Low = 1,
		Medium = 2,
		High = 3,
		Ultra = 4
	};

	struct RendererConfig {
		GraphicsQuality shadowQuality = GraphicsQuality::High;
		GraphicsQuality bloomQuality = GraphicsQuality::Medium;
		GraphicsQuality antiAliasing = GraphicsQuality::High;

		float bloomThreshold = 1.0f;
		float bloomBlurRadius = 3.0f;
		float bloomIntensity = 1.0f;
	};

	class Renderer {
	public:

		static void initialize(Window* window, std::function<void(Event&)> eventCallback);
		static void shutdown();

		static void prepareRendering();
		static void finishRendering();

		static void beginScene(const Camera& camera, const LightingData& lightingData);
		static void beginScene(const Mat4& projection, const Mat4& transform);
		static void endScene();

		static void setClearColor(RenderContext* renderContext, const Vec4& color);
		static void clear(RenderContext* renderContext);

		static void setRenderTarget(RenderContext* renderContext, FrameBuffer* target);
		static void restoreRenderTarget(RenderContext* renderContext);
		static void renderToSwapChain(RenderContext* renderContext);
		static FrameBuffer* getCurrentRenderTarget() { return s_currentRenderTarget; }

		static const Ref<ConstantBuffer>& getSceneDataBuffer();
		static uint32_t getSceneDataOffset();
		static double getFrameTimeMs() { return s_lastFrameTimeMs; }
		static Ref<Texture2D> getShadowMap(uint32_t cascadeIndex);
		static uint32_t getActiveShadowCascades();

		static const std::array<Mat4, 4>& getLightSpaceMatrices();
		static void calculateLightMatrices(const Camera& camera, const LightingData& lightingData);

		static void submit(RenderContext* renderContext, const Ref<Mesh>& mesh, const Ref<ConstantBuffer>& transform, const Ref<Shader>& shader, const Ref<ConstantBuffer>& uploadBuffer);

		static RendererStats& getStats();
		static void resetStats();

		static RendererConfig& getConfig();
		static void setConfig(const RendererConfig& config);

		inline static void setAPI(RendererAPI api) { s_api = api; }
		inline static RendererAPI getAPI() { return s_api; }

	private:

		static RendererAPI s_api;

		static RendererStats s_stats;
		static RendererConfig s_config;
		static FrameTimer s_frameTimer;
		static double s_lastFrameTimeMs;
		static std::function<void(Event&)> s_eventCallback;
		static thread_local uint32_t s_sceneDataOffset;
		static FrameBuffer* s_currentRenderTarget;
		static std::array<Ref<Texture2D>, 4> s_shadowMapTextures;
		static std::array<Mat4, 4> s_lightSpaceMatrices; // TODO change all 4s related to this to a constexpr or so

	};

}
