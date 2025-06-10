#include "axpch.h"
#include "Renderer.h"

#include "platform/directx/D12RendererAPI.h"

namespace Axion {

	RendererAPI* Renderer::s_rendererAPI = new D12RendererAPI();

	void Renderer::initialize(Window* window) {
		s_rendererAPI->initialize(window);
	}

	void Renderer::release() {
		s_rendererAPI->release();
		AX_CORE_LOG_INFO("Renderer shutdown");
	}

	void Renderer::beginScene(OrthographicCamera& camera) {
		s_rendererAPI->beginScene();
	}

	void Renderer::endScene() {
		s_rendererAPI->endScene();
	}

	void Renderer::clear(float r, float g, float b, float a) {
		s_rendererAPI->clear(r, g, b, a);
	}

	void Renderer::present() {
		s_rendererAPI->present();
	}

	void Renderer::submit(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib, const Ref<ConstantBuffer>& cb, uint32_t slot, const Ref<ConstantBuffer>& transform, uint32_t slotTransform) {
		cb->bind(slot);
		transform->bind(slotTransform);
		vb->bind();
		ib->bind();
		s_rendererAPI->drawIndexed(vb, ib);
	}

	void Renderer::submit(const Ref<Mesh>& mesh, const Ref<ConstantBuffer>& cb, uint32_t slot, const Ref<ConstantBuffer>& transform, uint32_t slotTransform) {
		cb->bind(slot);
		transform->bind(slotTransform);
		mesh->render();
		s_rendererAPI->drawIndexed(mesh->getVertexBuffer(), mesh->getIndexBuffer());
	}

}

