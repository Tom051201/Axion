#include "axpch.h"
#include "RenderCommand.h"

#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/render/GraphicsContext.h"

namespace Axion {

	uint32_t RenderCommand::s_drawCallCount = 0;

	void RenderCommand::setClearColor(const Vec4& color) {
		GraphicsContext::get()->setClearColor(color);
	}

	void RenderCommand::clear() {
		GraphicsContext::get()->clear();
	}

	void RenderCommand::drawIndexed(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib) {
		s_drawCallCount++;
		GraphicsContext::get()->drawIndexed(vb, ib);
	}

	void RenderCommand::drawIndexed(const Ref<IndexBuffer>& ib, uint32_t indexCount) {
		s_drawCallCount++;
		GraphicsContext::get()->drawIndexed(ib, indexCount);
	}

	void RenderCommand::resetRenderStats() {
		s_drawCallCount = 0;
	}

}
