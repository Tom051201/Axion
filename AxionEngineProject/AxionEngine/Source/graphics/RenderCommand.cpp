#include "axpch.h"
#include "RenderCommand.h"

#include "AxionEngine/Source/graphics/Renderer.h"
#include "AxionEngine/Source/graphics/GraphicsContext.h"

namespace Axion {

	void RenderCommand::setClearColor(const Vec4& color) {
		GraphicsContext::get()->setClearColor(color);
	}

	void RenderCommand::clear(RenderContext* renderContext) {
		GraphicsContext::get()->clear();
	}

	void RenderCommand::drawIndexed(RenderContext* renderContext, const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib, uint32_t instanceCount) {
		GraphicsContext::get()->drawIndexed(renderContext, vb, ib, instanceCount);
	}

	void RenderCommand::drawIndexed(RenderContext* renderContext, const Ref<IndexBuffer>& ib, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseIndexLocation) {
		GraphicsContext::get()->drawIndexed(renderContext, ib, indexCount, instanceCount, startIndexLocation, baseIndexLocation);
	}

	void RenderCommand::draw(RenderContext* renderContext, uint32_t vertexCount) {
		GraphicsContext::get()->draw(renderContext, vertexCount);
	}

}
