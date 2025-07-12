#pragma once

#include "AxionEngine/Source/render/Buffers.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// OpenGL3VertexBuffer //////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class OpenGL3VertexBuffer : public VertexBuffer {
	public:

		OpenGL3VertexBuffer(const std::vector<Vertex>& vertices);
		~OpenGL3VertexBuffer() override;

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
	///// OpenGL3IndexBuffer ///////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class OpenGL3IndexBuffer : public IndexBuffer {
	public:

		OpenGL3IndexBuffer(const std::vector<uint32_t>& indices);
		~OpenGL3IndexBuffer() override;

		void release() override;

		void bind() const override;
		void unbind() const override;

		uint32_t getIndexCount() const override { return m_indexCount; }

	private:

		uint32_t m_rendererID = 0;

		uint32_t m_indexCount = 0;

	};



	////////////////////////////////////////////////////////////////////////////////
	///// OpenGL3ConstantBuffer ////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class OpenGL3ConstantBuffer : public ConstantBuffer {
	public:

		OpenGL3ConstantBuffer(size_t size);
		~OpenGL3ConstantBuffer();

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
