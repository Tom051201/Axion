#include "axpch.h"
#include "DX12Mesh.h"

namespace Axion {

	DX12Mesh::DX12Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
		m_vertexBuffer = std::dynamic_pointer_cast<DX12VertexBuffer>(VertexBuffer::create(vertices));
		m_indexBuffer = std::dynamic_pointer_cast<DX12IndexBuffer>(IndexBuffer::create(indices));
	}

	DX12Mesh::DX12Mesh(const MeshData& meshData) {
		m_vertexBuffer = std::dynamic_pointer_cast<DX12VertexBuffer>(VertexBuffer::create(meshData.vertices));
		m_indexBuffer = std::dynamic_pointer_cast<DX12IndexBuffer>(IndexBuffer::create(meshData.indices));
		m_submeshes = meshData.submeshes;
	}

	DX12Mesh::~DX12Mesh() {
		release();
	}

	void DX12Mesh::release() {
		m_vertexBuffer->release();
		m_indexBuffer->release();
		m_submeshes.clear();
	}

	void DX12Mesh::render() const {
		m_vertexBuffer->bind();
		m_indexBuffer->bind();
	}

}
