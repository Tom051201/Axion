#pragma once
#include "axpch.h"

#include "AxionEngine/Source/core/Window.h"
#include "AxionEngine/Source/render/RendererAPI.h"
#include "AxionEngine/Source/render/OrthographicCamera.h"
#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Shader.h"

namespace Axion {

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

		inline static RendererAPI::API getAPI() { return RendererAPI::getAPI(); }
		inline static RendererAPI* getAPIInstance() { return s_rendererAPI; }

	private:

		static RendererAPI* s_rendererAPI;

	};

}
