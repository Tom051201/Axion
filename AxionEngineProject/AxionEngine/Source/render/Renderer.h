#pragma once
#include "axpch.h"

#include "AxionEngine/Source/core/Window.h"
#include "AxionEngine/Source/render/Camera.h"
#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Shader.h"

namespace Axion {

	enum class RendererAPI {
		None = 0,
		DirectX12 = 1,
		OpenGL3 = 2
	};

	class Renderer {
	public:

		static void initialize(Window* window, std::function<void(Event&)> eventCallback);
		static void release();

		static void prepareRendering();
		static void finishRendering();

		static void beginScene(const Camera& camera);
		static void beginScene(const Mat4& projection, const Mat4& transform);
		static void endScene();
		
		static void setClearColor(const Vec4& color);
		static void clear();

		static void renderToSwapChain();
		static const Ref<ConstantBuffer>& getSceneDataBuffer();
		static double getFrameTimeMs() { return s_lastFrameTimeMs; }

		static void submit(const Ref<Mesh>& mesh, const Ref<ConstantBuffer>& transform, const Ref<Shader>& shader, const Ref<ConstantBuffer>& uploadBuffer);

		inline static void setAPI(RendererAPI api) { s_api = api; }
		inline static RendererAPI getAPI() { return s_api; }

	private:
		
		static RendererAPI s_api;

		static FrameTimer s_frameTimer;
		static double s_lastFrameTimeMs;

	};

}
