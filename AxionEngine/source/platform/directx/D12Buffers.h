#pragma once

#include "axpch.h"

#include "Axion/render/Buffers.h"

#include "platform/directx/D12DataTypes.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// D12VertexBuffer //////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class D12VertexBuffer : public VertexBuffer {
	public:

		D12VertexBuffer(const std::vector<Vertex>& vertices);
		~D12VertexBuffer() override;

		void release() override;

		void bind() const override;
		void unbind() const override;

		inline void setLayout(const BufferLayout& layout) override { m_layout = layout; }
		inline const BufferLayout& getLayout() const override { return m_layout; }

		inline uint32_t getVertexCount() const override { return m_vertexCount; }
		inline uint32_t getSize() const override { return m_size; }
		
		inline const D3D12_VERTEX_BUFFER_VIEW& getView() const { return m_view; }

	private:

		Microsoft::WRL::ComPtr<ID3D12Resource> m_buffer;
		D3D12_VERTEX_BUFFER_VIEW m_view;

		uint32_t m_vertexCount = 0;
		uint32_t m_size = 0;
		uint32_t m_stride = sizeof(D12Vertex);
		BufferLayout m_layout;
	};

	////////////////////////////////////////////////////////////////////////////////
	///// D12IndexBuffer ///////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class D12IndexBuffer : public IndexBuffer {
	public:

		D12IndexBuffer(const std::vector<uint32_t>& indices);
		~D12IndexBuffer();

		void release() override;

		void bind() const override;
		void unbind() const override;

		inline uint32_t getIndexCount() const override { return m_indexCount; }

		inline const D3D12_INDEX_BUFFER_VIEW& getView() const { return m_view; }

	private:

		Microsoft::WRL::ComPtr<ID3D12Resource> m_buffer;
		D3D12_INDEX_BUFFER_VIEW m_view;

		uint32_t m_indexCount = 0;

	};

	////////////////////////////////////////////////////////////////////////////////
	///// D12ConstantBuffer ////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class D12ConstantBuffer : public ConstantBuffer {
	public:

		D12ConstantBuffer(size_t size);
		~D12ConstantBuffer() override;

		void release() override;

		void bind(uint32_t slot) const override;
		void unbind() const override;

		void update(const void* data, size_t size) override;

		inline uint32_t getSize() const override { return static_cast<uint32_t>(m_bufferSize); }

	private:

		Microsoft::WRL::ComPtr<ID3D12Resource> m_buffer;
		uint8_t* m_mappedPtr;
		size_t m_bufferSize = 0;

	};


}
