#pragma once

#include "Axion/core/Core.h"
#include "Axion/render/Mesh.h"

#include "platform/directx/D12Buffers.h"

namespace Axion {

	class D12Mesh : public Mesh {
	public:

		D12Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		~D12Mesh() override;

		void release() override;

		void render() override;

		Ref<VertexBuffer> getVertexBuffer() const override { return m_vertexBuffer; }
		Ref<IndexBuffer> getIndexBuffer() const override { return m_indexBuffer; }

		uint32_t getIndexCount() const override { return m_indexBuffer->getIndexCount(); }

	private:

		Ref<D12VertexBuffer> m_vertexBuffer;
		Ref<D12IndexBuffer> m_indexBuffer;

	};

}

