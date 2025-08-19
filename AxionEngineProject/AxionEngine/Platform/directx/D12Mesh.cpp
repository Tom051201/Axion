#include "axpch.h"
#include "D12Mesh.h"

namespace Axion {

	D12Mesh::D12Mesh(const AssetHandle<Mesh>& handle, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
		: m_handle(handle) {
		m_vertexBuffer = std::dynamic_pointer_cast<D12VertexBuffer>(VertexBuffer::create(vertices));
		m_indexBuffer = std::dynamic_pointer_cast<D12IndexBuffer>(IndexBuffer::create(indices));
	}

	D12Mesh::~D12Mesh() {
		release();
	}

	void D12Mesh::release() {
		m_vertexBuffer->release();
		m_indexBuffer->release();
	}

	void D12Mesh::render() const {
		m_vertexBuffer->bind();
		m_indexBuffer->bind();
	}

}
