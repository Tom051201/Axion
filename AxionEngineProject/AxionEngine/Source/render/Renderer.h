#pragma once
#include "axpch.h"

#include "AxionEngine/Source/core/Window.h"
#include "AxionEngine/Source/render/OrthographicCamera.h"
#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Shader.h"

namespace Axion {

	enum class RendererAPI {
		None = 0,
		Direct3D12 = 1,
		OpenGL = 2
	};

	class Renderer {
	public:

		static void initialize(Window* window);
		static void release();

		static void prepareRendering();
		static void finishRendering();

		static void beginScene(OrthographicCamera& camera);
		static void endScene();
		
		static void setClearColor(const Vec4& color);
		static void clear();

		static void submit(const Ref<Mesh>& mesh, const Ref<ConstantBuffer>& transform, const Ref<Shader>& shader);

		inline static void setAPI(RendererAPI api) { s_api = api; }
		inline static RendererAPI getAPI() { return s_api; }

	private:
		
		static RendererAPI s_api;

	};

}
