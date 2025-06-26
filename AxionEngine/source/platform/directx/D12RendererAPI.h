#pragma once
#include "Axion/render/RendererAPI.h"

#include "platform/directx/D12Context.h"

namespace Axion {

	class D12RendererAPI : public RendererAPI {
	public:

		void initialize(Window* window) override;
		void release() override;

		void prepareRendering() override;
		void finishRendering() override;

		void setClearColor(const Vec4& color);
		void clear() override;

		void drawIndexed(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib) override;

	private:

		D12Context* m_context;

	};

}
