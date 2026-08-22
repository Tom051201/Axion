#pragma once

#include <unordered_map>
#include <vector>
#include <d3d11.h>
#include <wrl/client.h>

#include "AxionEngine/Source/graphics/Buffers.h"

namespace Axion {

	class DX11VertexBuffer : public VertexBuffer {
	public:

		DX11VertexBuffer(const void* data, uint32_t size, uint32_t stride);
		DX11VertexBuffer(uint32_t size, uint32_t stride);
		~DX11VertexBuffer() override;

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

	private:

		BufferType m_type;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;

		std::vector<uint8_t> m_localData;
		mutable bool m_isDirty = false;

		uint32_t m_vertexCount = 0;
		uint32_t m_size = 0;
		uint32_t m_stride = sizeof(Vertex);
		BufferLayout m_layout;

		uint32_t m_currentOffset = 0;

	};





	class DX11IndexBuffer : public IndexBuffer {
	public:

		DX11IndexBuffer(const std::vector<uint32_t>& indices);
		DX11IndexBuffer(uint32_t maxIndices);
		~DX11IndexBuffer() override;

		void release() override;

		void bind() const override;
		void unbind() const override;

		uint32_t getIndexCount() const override { return m_indexCount; }

		void update(const void* data, size_t size) override;
		void update(const void* data, size_t size, size_t offset) override;

	private:

		BufferType m_type;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;

		std::vector<uint8_t> m_localData;
		mutable bool m_isDirty = false;

		uint32_t m_indexCount = 0;
		uint32_t m_size = 0;

	};





	class DX11ConstantBuffer : public ConstantBuffer {
	public:

		DX11ConstantBuffer(size_t size);
		~DX11ConstantBuffer() override;

		void release() override;

		void bind(uint32_t slot) const override;
		void bind(uint32_t slot, size_t offset) const override;
		void unbind() const override;

		void update(const void* data, size_t size) override;
		uint32_t append(const void* data, size_t size) override;
		void resetOffset() override;

		uint32_t getSize() const override { return static_cast<uint32_t>(m_bufferSize); }

	private:

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;

		std::vector<uint8_t> m_localData;
		mutable bool m_isDirty = false;

		size_t m_bufferSize = 0;
		uint32_t m_currentOffset = 0;

	};





	class DX11StructuredBuffer : public StructuredBuffer {
	public:

		DX11StructuredBuffer(uint32_t elementSize, uint32_t elementCount);
		~DX11StructuredBuffer() override;

		void release() override;

		void bind(uint32_t slot) const override;
		void bind(uint32_t slot, size_t offset) const override;
		void unbind() const override;

		void update(const void* data, size_t size) override;
		void update(const void* data, size_t size, size_t offset) override;
		uint32_t append(const void* data, size_t size) override;
		void resetOffset() override;

		uint32_t getSize() const override { return m_bufferSize; }
		uint32_t getElementCount() const override { return m_elementCount; }
		uint32_t getElementSize() const override { return m_elementSize; }

	private:

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;
		mutable std::unordered_map<size_t, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_srvs;

		std::vector<uint8_t> m_localData;
		mutable bool m_isDirty = false;

		uint32_t m_elementSize = 0;
		uint32_t m_elementCount = 0;
		uint32_t m_bufferSize = 0;
		uint32_t m_currentOffset = 0;

	};

}
