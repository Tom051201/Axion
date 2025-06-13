#pragma once
#include "axpch.h"

#include "Axion/core/Window.h"
#include "Axion/render/RendererAPI.h"
#include "Axion/render/OrthographicCamera.h"
#include "Axion/render/Mesh.h"
#include "Axion/render/Shader.h"

namespace Axion {

	class Renderer {
	public:

		static void initialize(Window* window);
		static void release();

		static void beginScene(OrthographicCamera& camera);
		static void endScene();
		
		static void setClearColor(const Vec4& color);
		static void clear();
		static void present();

		static void submit(const Ref<Mesh>& mesh, const Ref<ConstantBuffer>& transform, const Ref<Shader>& shader);

		inline static RendererAPI::API getAPI() { return RendererAPI::getAPI(); }
		inline static RendererAPI* getAPIInstance() { return s_rendererAPI; }

	private:

		static RendererAPI* s_rendererAPI;

	};

}
