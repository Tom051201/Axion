#pragma once

#include "AxionEngine/Source/render/Mesh.h"

#include "AxionEngine/Platform/opengl/OpenGL3Buffers.h"

namespace Axion {

	class OpenGL3Mesh : public Mesh {
	public:

		OpenGL3Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		~OpenGL3Mesh();

		void release();

		void render();

		Ref<VertexBuffer> getVertexBuffer() const override { return m_vertexBuffer; }
		Ref<IndexBuffer> getIndexBuffer() const override { return m_indexBuffer; }

		uint32_t getIndexCount() const override { return m_indexBuffer->getIndexCount(); }

	private:

		Ref<VertexBuffer> m_vertexBuffer;
		Ref<IndexBuffer> m_indexBuffer;

	};

}
