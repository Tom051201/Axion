#include "axpch.h"
#include "OpenGL3Buffers.h"

#include "AxionEngine/Vendor/glad/include/glad/glad.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// OpenGL3VertexBuffer //////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	OpenGL3VertexBuffer::OpenGL3VertexBuffer(const std::vector<Vertex>& vertices) {
		m_vertexCount = static_cast<uint32_t>(vertices.size());
		m_size = sizeof(Vertex) * m_vertexCount;

		glGenBuffers(1, &m_rendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
		glBufferData(GL_ARRAY_BUFFER, m_size, vertices.data(), GL_STATIC_DRAW);
	}

	OpenGL3VertexBuffer::~OpenGL3VertexBuffer() {
		release();
	}

	void OpenGL3VertexBuffer::release() {
		glDeleteBuffers(1, &m_rendererID);
		m_rendererID = 0;
	}

	void OpenGL3VertexBuffer::bind() const {
		glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
	}

	void OpenGL3VertexBuffer::unbind() const {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}



	////////////////////////////////////////////////////////////////////////////////
	///// OpenGL3IndexBuffer ///////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	OpenGL3IndexBuffer::OpenGL3IndexBuffer(const std::vector<uint32_t>& indices) {
		m_indexCount = static_cast<uint32_t>(indices.size());

		glGenBuffers(1, &m_rendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * m_indexCount, indices.data(), GL_STATIC_DRAW);
	}

	OpenGL3IndexBuffer::~OpenGL3IndexBuffer() {
		release();
	}

	void OpenGL3IndexBuffer::release() {
		glDeleteBuffers(1, &m_rendererID);
		m_rendererID = 0;
	}

	void OpenGL3IndexBuffer::bind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);
	}

	void OpenGL3IndexBuffer::unbind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}



	////////////////////////////////////////////////////////////////////////////////
	///// OpenGL3ConstantBuffer ////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	OpenGL3ConstantBuffer::OpenGL3ConstantBuffer(size_t size)
		: m_size(static_cast<uint32_t>(size)) {

		glGenBuffers(1, &m_rendererID);
		glBindBuffer(GL_UNIFORM_BUFFER, m_rendererID);
		glBufferData(GL_UNIFORM_BUFFER, m_size, nullptr, GL_DYNAMIC_DRAW);
	}

	OpenGL3ConstantBuffer::~OpenGL3ConstantBuffer() {
		release();
	}

	void OpenGL3ConstantBuffer::release() {
		glDeleteBuffers(1, &m_rendererID);
		m_rendererID = 0;
	}

	void OpenGL3ConstantBuffer::update(const void* data, size_t size) {
		glBindBuffer(GL_UNIFORM_BUFFER, m_rendererID);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
	}

	void OpenGL3ConstantBuffer::bind(uint32_t slot) const {
		glBindBufferBase(GL_UNIFORM_BUFFER, slot, m_rendererID);
	}

	void OpenGL3ConstantBuffer::unbind() const {
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

}
