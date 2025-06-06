#pragma once

#include "axpch.h"
#include "Axion/Window.h"
#include "Axion/render/Buffers.h"

namespace Axion {

	class RendererAPI {
	public:

		enum class API {
			None = 0,
			Direct3D12 = 1
		};

		virtual void initialize(Window* window) = 0;
		virtual void release() = 0;

		virtual void beginScene() = 0;
		virtual void endScene() = 0;
		virtual void clear(float r, float g, float b, float a) = 0;
		virtual void present() = 0;

		virtual void drawIndexed(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib) = 0;

		inline static API getAPI() { return s_api; }

	private:

		static API s_api;

	};

}
