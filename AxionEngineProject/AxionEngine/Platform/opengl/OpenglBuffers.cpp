#include "axpch.h"
#include "OpenglBuffers.h"

#include "AxionEngine/Vendor/glad/include/glad/glad.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// OpenglVertexBuffer ///////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	OpenglVertexBuffer::OpenglVertexBuffer(const std::vector<Vertex>& vertices) {
		m_vertexCount = static_cast<uint32_t>(vertices.size());
		m_size = sizeof(Vertex) * m_vertexCount;

		glGenBuffers(1, &m_rendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
		glBufferData(GL_ARRAY_BUFFER, m_size, vertices.data(), GL_STATIC_DRAW);
	}

	OpenglVertexBuffer::~OpenglVertexBuffer() {
		release();
	}

	void OpenglVertexBuffer::release() {
		glDeleteBuffers(1, &m_rendererID);
		m_rendererID = 0;
	}

	void OpenglVertexBuffer::bind() const {
		glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
	}

	void OpenglVertexBuffer::unbind() const {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}



	////////////////////////////////////////////////////////////////////////////////
	///// OpenglIndexbuffer ////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	OpenglIndexBuffer::OpenglIndexBuffer(const std::vector<uint32_t>& indices) {
		m_indexCount = static_cast<uint32_t>(indices.size());

		glGenBuffers(1, &m_rendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * m_indexCount, indices.data(), GL_STATIC_DRAW);
	}

	OpenglIndexBuffer::~OpenglIndexBuffer() {
		release();
	}

	void OpenglIndexBuffer::release() {
		glDeleteBuffers(1, &m_rendererID);
		m_rendererID = 0;
	}

	void OpenglIndexBuffer::bind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);
	}

	void OpenglIndexBuffer::unbind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}



	////////////////////////////////////////////////////////////////////////////////
	///// OpenglConstantBuffer /////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	OpenglConstantBuffer::OpenglConstantBuffer(size_t size)
		: m_size(static_cast<uint32_t>(size)) {

		glGenBuffers(1, &m_rendererID);
		glBindBuffer(GL_UNIFORM_BUFFER, m_rendererID);
		glBufferData(GL_UNIFORM_BUFFER, m_size, nullptr, GL_DYNAMIC_DRAW);
	}

	OpenglConstantBuffer::~OpenglConstantBuffer() {
		release();
	}

	void OpenglConstantBuffer::release() {
		glDeleteBuffers(1, &m_rendererID);
		m_rendererID = 0;
	}

	void OpenglConstantBuffer::update(const void* data, size_t size) {
		glBindBuffer(GL_UNIFORM_BUFFER, m_rendererID);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
	}

	void OpenglConstantBuffer::bind(uint32_t slot) const {
		glBindBufferBase(GL_UNIFORM_BUFFER, slot, m_rendererID);
	}

	void OpenglConstantBuffer::unbind() const {
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}


}
