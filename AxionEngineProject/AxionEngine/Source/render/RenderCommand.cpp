#include "axpch.h"
#include "RenderCommand.h"

#include "AxionEngine/Source/render/Renderer.h"

namespace Axion {

	void RenderCommand::setClearColor(const Vec4& color) {
		Renderer::getAPIInstance()->setClearColor(color);
	}

	void RenderCommand::clear() {
		Renderer::getAPIInstance()->clear();
	}

	void RenderCommand::drawIndexed(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib) {
		Renderer::getAPIInstance()->drawIndexed(vb, ib);
	}

}
