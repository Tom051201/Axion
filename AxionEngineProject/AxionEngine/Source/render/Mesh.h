#pragma once

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/render/Buffers.h"
#include "AxionEngine/Source/core/AssetHandle.h"

namespace Axion {

	struct Submesh {
		uint32_t baseVertex;
		uint32_t startIndex;
		uint32_t indexCount;
		uint32_t materialIndex;
	};

	struct MeshData {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<Submesh> submeshes;
	};

	class Mesh {
	public:

		Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		Mesh(const MeshData& meshData);
		~Mesh();

		void release();

		Ref<VertexBuffer> getVertexBuffer() const { return m_vertexBuffer; }
		Ref<IndexBuffer> getIndexBuffer() const { return m_indexBuffer; }
		uint32_t getVertexCount() const { return m_vertexBuffer->getVertexCount(); }
		uint32_t getIndexCount() const { return m_indexBuffer->getIndexCount(); }
		const std::vector<Submesh>& getSubmeshes() const { return m_submeshes; }




		static Ref<Mesh> create(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		static Ref<Mesh> create(const MeshData& meshData);

		static Ref<Mesh> createPBRCube();

	private:

		Ref<VertexBuffer> m_vertexBuffer;
		Ref<IndexBuffer> m_indexBuffer;
		std::vector<Submesh> m_submeshes;

	};

}
