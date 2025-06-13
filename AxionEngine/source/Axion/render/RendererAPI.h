#pragma once
#include "axpch.h"

#include "Axion/core/Window.h"
#include "Axion/core/Math.h"
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

		virtual void setClearColor(const Vec4& color) = 0;
		virtual void clear() = 0;
		virtual void present() = 0;

		virtual void drawIndexed(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib) = 0;

		inline static API getAPI() { return s_api; }

	private:

		static API s_api;

	};

}
