#pragma once
#include "axpch.h"

#include "Axion/core/Window.h"
#include "Axion/render/RendererAPI.h"
#include "Axion/render/OrthographicCamera.h"
#include "Axion/render/Mesh.h"

namespace Axion {

	class Renderer {
	public:

		static void initialize(Window* window);
		static void release();

		static void beginScene(OrthographicCamera& camera);
		static void endScene();
		
		static void clear(float r, float g, float b, float a);
		static void present();

		static void submit(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib, const Ref<ConstantBuffer>& cb, uint32_t slot, const Ref<ConstantBuffer>& transform, uint32_t slotTransform);
		static void submit(const Ref<Mesh>& mesh, const Ref<ConstantBuffer>& cb, uint32_t slot, const Ref<ConstantBuffer>& transform, uint32_t slotTransform);

		inline static RendererAPI::API getAPI() { return RendererAPI::getAPI(); }

	private:

		static RendererAPI* s_rendererAPI;

	};

}
