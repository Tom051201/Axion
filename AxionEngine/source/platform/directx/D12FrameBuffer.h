#pragma once
#include "axpch.h"

#include "Axion/render/FrameBuffer.h"

namespace Axion {

	class D12Context;


	class D12FrameBuffer : public FrameBuffer {
	public:

		D12FrameBuffer(const FrameBufferSpecification& spec);
		~D12FrameBuffer() override;

		void release() override;
		void resize(uint32_t width, uint32_t height) override;

		void bind() const override;
		void unbind() const override;

		void clear() override;
		void clear(const Vec4& clearColor) override;

		const FrameBufferSpecification& getSpecification() const override { return m_specification; }

		uint32_t getRtvHeapIndex() const { return m_rtvHeapIndex; }
		uint32_t getSrvHeapIndex() const { return m_srvHeapIndex; }
		ID3D12Resource* getColorResource() const { return m_colorResource.Get(); }

	private:

		D12Context* m_context = nullptr;

		FrameBufferSpecification m_specification;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_colorResource;
		uint32_t m_rtvHeapIndex = 0;
		uint32_t m_srvHeapIndex = 0;
		mutable D3D12_RESOURCE_STATES m_currentState = D3D12_RESOURCE_STATE_RENDER_TARGET;

	};

}
