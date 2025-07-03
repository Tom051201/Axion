#include "axpch.h"
#include "D12RendererAPI.h"

#include "AxionEngine/Source/render/GraphicsContext.h"

namespace Axion {

	void D12RendererAPI::initialize(Window* window) {
		m_context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());
		m_context->initialize((HWND)window->getNativeHandle(), window->getWidth(), window->getHeight());
	}

	void D12RendererAPI::release() {
		// Does nothing for now
	}

	void D12RendererAPI::prepareRendering() {
		m_context->prepareRendering();
	}

	void D12RendererAPI::finishRendering() {
		m_context->finishRendering();
	}

	void D12RendererAPI::setClearColor(const Vec4& color) {
		m_context->setClearColor(color);
	}

	void D12RendererAPI::clear() {
		m_context->clear();
	}

	void D12RendererAPI::drawIndexed(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib) {
		m_context->getCommandList()->DrawIndexedInstanced(ib->getIndexCount(), 1, 0, 0, 0);
	}

}
