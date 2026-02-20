#pragma once

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/render/Buffers.h"

namespace Axion {

	class RenderCommand {
	public:

		static void setClearColor(const Vec4& color);
		static void clear();

		static void drawIndexed(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib, uint32_t instanceCount = 1);
		static void drawIndexed(const Ref<IndexBuffer>& ib, uint32_t indexCount, uint32_t instanceCount = 1);

		static void resetRenderStats();
		static uint32_t getDrawCallCount() { return s_drawCallCount; }

	private:

		// render stats
		static uint32_t s_drawCallCount;

	};

}
