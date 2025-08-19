#pragma once

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/render/Mesh.h"

#include "AxionEngine/Platform/directx/D12Buffers.h"

namespace Axion {

	class D12Mesh : public Mesh {
	public:

		D12Mesh(const AssetHandle<Mesh>& handle, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		~D12Mesh() override;

		void release() override;

		void render() const override;

		Ref<VertexBuffer> getVertexBuffer() const override { return m_vertexBuffer; }
		Ref<IndexBuffer> getIndexBuffer() const override { return m_indexBuffer; }

		uint32_t getIndexCount() const override { return m_indexBuffer->getIndexCount(); }

		const AssetHandle<Mesh>& getHandle() const { return m_handle; }

	private:

		const AssetHandle<Mesh> m_handle;
		Ref<D12VertexBuffer> m_vertexBuffer;
		Ref<D12IndexBuffer> m_indexBuffer;

	};

}

