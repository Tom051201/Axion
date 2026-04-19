#pragma once

#include "AxionEngine/Source/render/Buffers.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// DX12VertexBuffer /////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class DX12VertexBuffer : public VertexBuffer {
	public:

		DX12VertexBuffer(const std::vector<Vertex>& vertices);
		DX12VertexBuffer(uint32_t size, uint32_t stride);
		~DX12VertexBuffer() override;

		void release() override;

		void bind(uint32_t slot = 0, uint32_t offset = 0) const override;
		void unbind() const override;

		void setLayout(const BufferLayout& layout) override { m_layout = layout; }
		const BufferLayout& getLayout() const override { return m_layout; }

		uint32_t getVertexCount() const override { return m_vertexCount; }
		uint32_t getSize() const override { return m_size; }

		void update(const void* data, size_t size) override;
		void update(const void* data, size_t size, size_t offset) override;
		uint32_t append(const void* data, size_t size) override;
		void resetOffset() override;

		const D3D12_VERTEX_BUFFER_VIEW& getView() const { return m_view; }

	private:

		BufferType m_type;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_buffer;
		D3D12_VERTEX_BUFFER_VIEW m_view;

		uint8_t* m_mappedPtr;
		uint32_t m_vertexCount = 0;
		uint32_t m_size = 0;
		uint32_t m_stride = sizeof(Vertex);
		BufferLayout m_layout;

		uint32_t m_currentOffset = 0;
	};

	////////////////////////////////////////////////////////////////////////////////
	///// DX12IndexBuffer //////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class DX12IndexBuffer : public IndexBuffer {
	public:

		DX12IndexBuffer(const std::vector<uint32_t>& indices);
		DX12IndexBuffer(uint32_t maxIndices);
		~DX12IndexBuffer();

		void release() override;

		void bind() const override;
		void unbind() const override;

		uint32_t getIndexCount() const override { return m_indexCount; }

		void update(const void* data, size_t size) override;
		void update(const void* data, size_t size, size_t offset) override;

		const D3D12_INDEX_BUFFER_VIEW& getView() const { return m_view; }

	private:

		BufferType m_type;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_buffer;
		D3D12_INDEX_BUFFER_VIEW m_view;

		uint8_t* m_mappedPtr;
		uint32_t m_indexCount = 0;

	};

	////////////////////////////////////////////////////////////////////////////////
	///// DX12ConstantBuffer ///////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class DX12ConstantBuffer : public ConstantBuffer {
	public:

		DX12ConstantBuffer(size_t size);
		~DX12ConstantBuffer() override;

		void release() override;

		void bind(uint32_t slot) const override;
		void bind(uint32_t slot, size_t offset) const override;
		void unbind() const override;

		void update(const void* data, size_t size) override;
		uint32_t append(const void* data, size_t size) override;
		void resetOffset() override;

		uint32_t getSize() const override { return static_cast<uint32_t>(m_bufferSize); }

	private:

		Microsoft::WRL::ComPtr<ID3D12Resource> m_buffer;
		uint8_t* m_mappedPtr;
		size_t m_bufferSize = 0;

		uint32_t m_currentOffset = 0;

	};


}
