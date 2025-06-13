#pragma once

#include "Axion/core/Core.h"
#include "Axion/core/Math.h"
#include "Axion/render/Buffers.h"

namespace Axion {

	class RenderCommand {
	public:

		static void setClearColor(const Vec4& color);
		static void clear();

		static void drawIndexed(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib);

	private:

		static Vec4 s_clearColor;

	};

}
