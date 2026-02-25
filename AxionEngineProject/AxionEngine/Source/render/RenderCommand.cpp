#include "axpch.h"
#include "RenderCommand.h"

#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/render/GraphicsContext.h"

namespace Axion {

	void RenderCommand::setClearColor(const Vec4& color) {
		GraphicsContext::get()->setClearColor(color);
	}

	void RenderCommand::clear() {
		GraphicsContext::get()->clear();
	}

	void RenderCommand::drawIndexed(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib, uint32_t instanceCount) {
		GraphicsContext::get()->drawIndexed(vb, ib, instanceCount);
	}

	void RenderCommand::drawIndexed(const Ref<IndexBuffer>& ib, uint32_t indexCount, uint32_t instanceCount) {
		GraphicsContext::get()->drawIndexed(ib, indexCount, instanceCount);
	}

	void RenderCommand::draw(uint32_t vertexCount) {
		GraphicsContext::get()->draw(vertexCount);
	}

}
