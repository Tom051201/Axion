#include "axpch.h"
#include "OpenGL3Mesh.h"

namespace Axion {

	OpenGL3Mesh::OpenGL3Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
		m_vertexBuffer = std::dynamic_pointer_cast<OpenGL3VertexBuffer>(VertexBuffer::create(vertices));
		m_indexBuffer = std::dynamic_pointer_cast<OpenGL3IndexBuffer>(IndexBuffer::create(indices));
	}

	OpenGL3Mesh::~OpenGL3Mesh() {
		release();
	}

	void OpenGL3Mesh::release() {
		m_vertexBuffer->release();
		m_indexBuffer->release();
	}

	void OpenGL3Mesh::render() {
		m_vertexBuffer->bind();
		m_indexBuffer->bind();
	}

}
