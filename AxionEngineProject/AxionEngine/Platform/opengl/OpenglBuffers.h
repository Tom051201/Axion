#pragma once

#include "AxionEngine/Source/render/Buffers.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// OpenglVertexBuffer ///////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class OpenglVertexBuffer : public VertexBuffer {
	public:

		OpenglVertexBuffer(const std::vector<Vertex>& vertices);
		~OpenglVertexBuffer() override;

		void release() override;

		void bind() const override;
		void unbind() const override;

		inline void setLayout(const BufferLayout& layout) override { m_layout = layout; }
		inline const BufferLayout& getLayout() const override { return m_layout; }

		inline uint32_t getVertexCount() const override { return m_vertexCount; }
		inline uint32_t getSize() const override { return m_size; }

	private:

		uint32_t m_rendererID = 0;

		uint32_t m_vertexCount = 0;
		uint32_t m_size = 0;
		BufferLayout m_layout;

	};



	////////////////////////////////////////////////////////////////////////////////
	///// OpenglIndexbuffer ////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class OpenglIndexBuffer : public IndexBuffer {
	public:

		OpenglIndexBuffer(const std::vector<uint32_t>& indices);
		~OpenglIndexBuffer() override;

		void release() override;

		void bind() const override;
		void unbind() const override;

		uint32_t getIndexCount() const override { return m_indexCount; }

	private:

		uint32_t m_rendererID = 0;

		uint32_t m_indexCount = 0;

	};



	////////////////////////////////////////////////////////////////////////////////
	///// OpenglConstantBuffer /////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class OpenglConstantBuffer : public ConstantBuffer {
	public:

		OpenglConstantBuffer(size_t size);
		~OpenglConstantBuffer();

		void release() override;

		void bind(uint32_t slot) const override;
		void unbind() const override;

		void update(const void* data, size_t size) override;
		uint32_t getSize() const override { return m_size; }

	private:

		uint32_t m_rendererID = 0;

		uint32_t m_size = 0;

	};

}
