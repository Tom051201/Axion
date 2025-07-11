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

	void RenderCommand::drawIndexed(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib) {
		GraphicsContext::get()->drawIndexed(vb, ib);
	}

}
