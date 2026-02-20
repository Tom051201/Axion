#include "axpch.h"
#include "OpenGL3Buffers.h"

#include "AxionEngine/Vendor/glad/include/glad/glad.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// OpenGL3VertexBuffer //////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	// Static constructor
	OpenGL3VertexBuffer::OpenGL3VertexBuffer(const std::vector<Vertex>& vertices) {
		m_type = BufferType::Static;
		m_vertexCount = static_cast<uint32_t>(vertices.size());
		m_size = m_vertexCount * sizeof(Vertex);

		glGenBuffers(1, &m_rendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
		glBufferData(GL_ARRAY_BUFFER, m_size, vertices.data(), GL_STATIC_DRAW);
	}

	// Dynamic constuctor
	OpenGL3VertexBuffer::OpenGL3VertexBuffer(uint32_t size, uint32_t stride) {
		m_type = BufferType::Dynamic;
		m_vertexCount = 0;
		m_size = size;
		m_stride = stride;

		glGenBuffers(1, &m_rendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	OpenGL3VertexBuffer::~OpenGL3VertexBuffer() {
		release();
	}

	void OpenGL3VertexBuffer::release() {
		if (!m_rendererID) return;

		glDeleteBuffers(1, &m_rendererID);
		m_rendererID = 0;
	}

	void OpenGL3VertexBuffer::bind(uint32_t slot, uint32_t offset) const {
		glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
	}

	void OpenGL3VertexBuffer::unbind() const {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void OpenGL3VertexBuffer::update(const void* data, size_t size) {
		AX_CORE_ASSERT(m_type == BufferType::Dynamic, "Updating a static OpenGL vertex buffer");
		AX_CORE_ASSERT(size <= m_size, "OpenGL vertex buffer overflow");

		glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}

	void OpenGL3VertexBuffer::update(const void* data, size_t size, size_t offset) {
		AX_CORE_ASSERT(m_type == BufferType::Dynamic, "Updating a static OpenGL vertex buffer");
		AX_CORE_ASSERT(offset + size <= m_size, "OpenGL vertex buffer overflow");

		glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
		glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
	}



	////////////////////////////////////////////////////////////////////////////////
	///// OpenGL3IndexBuffer ///////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	// Static constructor
	OpenGL3IndexBuffer::OpenGL3IndexBuffer(const std::vector<uint32_t>& indices) {
		m_type = BufferType::Static;
		m_indexCount = static_cast<uint32_t>(indices.size());

		glGenBuffers(1, &m_rendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexCount * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
	}

	OpenGL3IndexBuffer::OpenGL3IndexBuffer(uint32_t maxIndices) {
		m_type = BufferType::Dynamic;
		m_indexCount = 0;

		glGenBuffers(1, &m_rendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, maxIndices * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
	}

	OpenGL3IndexBuffer::~OpenGL3IndexBuffer() {
		release();
	}

	void OpenGL3IndexBuffer::release() {
		if (!m_rendererID) return;

		glDeleteBuffers(1, &m_rendererID);
		m_rendererID = 0;
	}

	void OpenGL3IndexBuffer::bind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);
	}

	void OpenGL3IndexBuffer::unbind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void OpenGL3IndexBuffer::update(const void* data, size_t size) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size, data);
	}

	void OpenGL3IndexBuffer::update(const void* data, size_t size, size_t offset) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, data);
	}



	////////////////////////////////////////////////////////////////////////////////
	///// OpenGL3ConstantBuffer ////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	OpenGL3ConstantBuffer::OpenGL3ConstantBuffer(size_t size) {
		GLint alignment = 0;
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);

		m_size = static_cast<uint32_t>((size + alignment - 1) & ~(alignment - 1));

		glGenBuffers(1, &m_rendererID);
		glBindBuffer(GL_UNIFORM_BUFFER, m_rendererID);
		glBufferData(GL_UNIFORM_BUFFER, m_size, nullptr, GL_DYNAMIC_DRAW);
	}

	OpenGL3ConstantBuffer::~OpenGL3ConstantBuffer() {
		release();
	}

	void OpenGL3ConstantBuffer::release() {
		if (!m_rendererID) return;

		glDeleteBuffers(1, &m_rendererID);
		m_rendererID = 0;
	}

	void OpenGL3ConstantBuffer::update(const void* data, size_t size) {
		AX_CORE_ASSERT(size <= m_size, "OpenGL constant buffer overflow");

		glBindBuffer(GL_UNIFORM_BUFFER, m_rendererID);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
	}

	void OpenGL3ConstantBuffer::bind(uint32_t slot) const {
		glBindBufferBase(GL_UNIFORM_BUFFER, slot, m_rendererID);
	}

	void OpenGL3ConstantBuffer::bind(uint32_t slot, size_t offset) const {
		AX_CORE_ASSERT(offset + 256 <= m_size, "OpenGL constant buffer bounds check failed");

		glBindBufferRange(GL_UNIFORM_BUFFER, slot, m_rendererID, offset, m_size - offset);
	}

	void OpenGL3ConstantBuffer::unbind() const {
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

}
