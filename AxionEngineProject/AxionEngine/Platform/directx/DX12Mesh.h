#pragma once

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/render/Mesh.h"

#include "AxionEngine/Platform/directx/DX12Buffers.h"

namespace Axion {

	class DX12Mesh : public Mesh {
	public:

		DX12Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		DX12Mesh(const MeshData& meshData);
		~DX12Mesh() override;

		void release() override;

		void render() const override;

		Ref<VertexBuffer> getVertexBuffer() const override { return m_vertexBuffer; }
		Ref<IndexBuffer> getIndexBuffer() const override { return m_indexBuffer; }
		const std::vector<Submesh>& getSubmeshes() const override { return m_submeshes; }

		uint32_t getIndexCount() const override { return m_indexBuffer->getIndexCount(); }

	private:

		const AssetHandle<Mesh> m_handle;
		Ref<DX12VertexBuffer> m_vertexBuffer;
		Ref<DX12IndexBuffer> m_indexBuffer;
		std::vector<Submesh> m_submeshes;

	};

}

