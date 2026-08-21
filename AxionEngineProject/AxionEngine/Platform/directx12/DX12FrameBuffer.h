#pragma once

#include <cstdint>
#include <d3d12.h>
#include <wrl/client.h>

#include "AxionEngine/Source/graphics/FrameBuffer.h"

namespace Axion {

	class DX12Context;


	class DX12FrameBuffer : public FrameBuffer {
	public:

		DX12FrameBuffer(const FrameBufferSpecification& spec);
		~DX12FrameBuffer() override;

		void release() override;
		void resize(uint32_t width, uint32_t height) override;

		void bind() const override;
		void unbind() const override;

		void clear() override;
		void clear(const Vec4& clearColor) override;
		void clearDepth() override;

		void clearAttachment(uint32_t attachmentIndex, int value) override;
		int readPixel(uint32_t attachementIndex, int x, int y);

		void* getColorAttachmentHandle() const override;
		void* getColorAttachmentNativeResource() const override { return (void*)m_colorResource.Get(); }
		const FrameBufferSpecification& getSpecification() const override { return m_specification; }

		uint32_t getRtvHeapIndex() const { return m_rtvHeapIndex; }
		uint32_t getSrvHeapIndex() const { return m_srvHeapIndex; }
		uint32_t getDsvHeapIndex() const { return m_dsvHeapIndex; }
		ID3D12Resource* getColorResource() const { return m_colorResource.Get(); }
		ID3D12Resource* getDepthResource() const { return m_depthResource.Get(); }

	private:

		DX12Context* m_context = nullptr;

		FrameBufferSpecification m_specification;
		bool m_allocated = false;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_colorResource;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_depthResource;

		// -- Entity ID Resources --
		Microsoft::WRL::ComPtr<ID3D12Resource> m_entityIdResource;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_readbackBuffer;
		uint32_t m_entityIdRtvHeapIndex = 0;
		mutable D3D12_RESOURCE_STATES m_entityIdState;

		uint32_t m_rtvHeapIndex = 0;
		uint32_t m_srvHeapIndex = 0;
		uint32_t m_dsvHeapIndex = 0;
		
		mutable D3D12_RESOURCE_STATES m_currentState;

	};

}
