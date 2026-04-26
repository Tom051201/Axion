#include "axpch.h"
#include "Mesh.h"

#include "AxionEngine/Source/render/Renderer.h"

namespace Axion {

	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
		m_vertexBuffer = VertexBuffer::create(vertices);
		m_indexBuffer = IndexBuffer::create(indices);
	}

	Mesh::Mesh(const MeshData& meshData) {
		m_vertexBuffer = VertexBuffer::create(meshData.vertices);
		m_indexBuffer = IndexBuffer::create(meshData.indices);
		m_submeshes = meshData.submeshes;
	}

	Mesh::~Mesh() {
		release();
	}

	void Mesh::release() {
		m_vertexBuffer->release();
		m_indexBuffer->release();
		m_submeshes.clear();
	}

	Ref<Mesh> Mesh::create(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
		return std::make_shared<Mesh>(vertices, indices);
	}

	Ref<Mesh> Mesh::create(const MeshData& meshData) {
		return std::make_shared<Mesh>(meshData);
	}


	// ----- Create Function for a 1x1 cube -----
	Ref<Mesh> Mesh::createPBRCube() {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		auto addVert = [&](float px, float py, float pz, float nx, float ny, float nz, float tx, float ty, float tz, float u, float v) {
			Vertex vert;
			vert.position = { px, py, pz };
			vert.normal = { nx, ny, nz };
			vert.tangent = { tx, ty, tz };
			vert.texcoord = { u, v };
			vertices.push_back(vert);
		};

		// -- Front Face --
		addVert(-1, 1, -1, 0, 0, -1, 1, 0, 0, 0, 0);	// Top Left
		addVert(1, 1, -1, 0, 0, -1, 1, 0, 0, 1, 0);		// Top Right
		addVert(1, -1, -1, 0, 0, -1, 1, 0, 0, 1, 1);	// Bottom Right
		addVert(-1, -1, -1, 0, 0, -1, 1, 0, 0, 0, 1);	// Bottom Left

		// -- Back Face --
		addVert(1, 1, 1, 0, 0, 1, -1, 0, 0, 0, 0);
		addVert(-1, 1, 1, 0, 0, 1, -1, 0, 0, 1, 0);
		addVert(-1, -1, 1, 0, 0, 1, -1, 0, 0, 1, 1);
		addVert(1, -1, 1, 0, 0, 1, -1, 0, 0, 0, 1);

		// -- Left Face --
		addVert(-1, 1, 1, -1, 0, 0, 0, 0, 1, 0, 0);
		addVert(-1, 1, -1, -1, 0, 0, 0, 0, 1, 1, 0);
		addVert(-1, -1, -1, -1, 0, 0, 0, 0, 1, 1, 1);
		addVert(-1, -1, 1, -1, 0, 0, 0, 0, 1, 0, 1);

		// -- Right Face --
		addVert(1, 1, -1, 1, 0, 0, 0, 0, -1, 0, 0);
		addVert(1, 1, 1, 1, 0, 0, 0, 0, -1, 1, 0);
		addVert(1, -1, 1, 1, 0, 0, 0, 0, -1, 1, 1);
		addVert(1, -1, -1, 1, 0, 0, 0, 0, -1, 0, 1);

		// -- Top Face --
		addVert(-1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0);
		addVert(1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0);
		addVert(1, 1, -1, 0, 1, 0, 1, 0, 0, 1, 1);
		addVert(-1, 1, -1, 0, 1, 0, 1, 0, 0, 0, 1);

		// -- Bottom Face --
		addVert(-1, -1, -1, 0, -1, 0, 1, 0, 0, 0, 0);
		addVert(1, -1, -1, 0, -1, 0, 1, 0, 0, 1, 0);
		addVert(1, -1, 1, 0, -1, 0, 1, 0, 0, 1, 1);
		addVert(-1, -1, 1, 0, -1, 0, 1, 0, 0, 0, 1);

		// -- Indices --
		uint32_t offset = 0;
		for (int i = 0; i < 6; i++) {
			indices.push_back(offset + 0);
			indices.push_back(offset + 1);
			indices.push_back(offset + 2);

			indices.push_back(offset + 2);
			indices.push_back(offset + 3);
			indices.push_back(offset + 0);
			offset += 4;
		}

		return create(vertices, indices);

	}

}
