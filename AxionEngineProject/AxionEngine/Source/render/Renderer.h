#pragma once
#include "axpch.h"

#include "AxionEngine/Source/core/Window.h"
#include "AxionEngine/Source/render/Camera.h"
#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Shader.h"
#include "AxionEngine/Source/render/Texture.h"

namespace Axion {

	enum class RendererAPI {
		None = 0,
		DirectX12 = 1,
		OpenGL3 = 2
	};

	constexpr uint32_t MAX_DIR_LIGHTS = 4;
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

		std::vector<DirectionalLightData> directionalLights;
		std::vector<PointLightData> pointLights;
		std::vector<SpotLightData> spotLights;
	};

	struct RendererStats {
		uint32_t drawCalls = 0;
		uint32_t quadCount2D = 0;
		uint32_t meshCount3D = 0; // unique meshes drawn
		uint32_t instanceCount3D = 0; // total 3d objects drawn

		uint32_t getTotalVertexCount2D() const { return quadCount2D * 4; }
		uint32_t getTotalIndexCount2D() const { return quadCount2D * 6; }
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

		static void setClearColor(const Vec4& color);
		static void clear();

		static void renderToSwapChain();
		static const Ref<ConstantBuffer>& getSceneDataBuffer();
		static double getFrameTimeMs() { return s_lastFrameTimeMs; }
		static void bindTextures(const std::array<Ref<Texture2D>, 16>& textures, uint32_t count, uint32_t rootIndex = 2);

		static const Ref<Texture2D>& getWhiteFallbackTexture();

		static void submit(const Ref<Mesh>& mesh, const Ref<ConstantBuffer>& transform, const Ref<Shader>& shader, const Ref<ConstantBuffer>& uploadBuffer);

		static RendererStats& getStats();
		static void resetStats();

		inline static void setAPI(RendererAPI api) { s_api = api; }
		inline static RendererAPI getAPI() { return s_api; }

	private:

		static RendererAPI s_api;

		static RendererStats s_stats;
		static FrameTimer s_frameTimer;
		static double s_lastFrameTimeMs;
		static Ref<Texture2D> s_whiteFallbackTexture;
		static std::function<void(Event&)> s_eventCallback;

	};

}
