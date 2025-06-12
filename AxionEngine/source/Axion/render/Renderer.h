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
		
		static void clear(float r, float g, float b, float a);
		static void present();

		static void submit(const Ref<Mesh>& mesh, const Ref<ConstantBuffer>& transform, const Ref<Shader>& shader);

		inline static RendererAPI::API getAPI() { return RendererAPI::getAPI(); }

	private:

		static RendererAPI* s_rendererAPI;

	};

}
