#pragma once

#include "Axion/Core.h"

#include "Buffers.h"
#include "DataTypes.h"

namespace Axion {

	class Mesh {
	public:

		virtual ~Mesh() = default;

		virtual void release() = 0;

		virtual void render() = 0;

		virtual Ref<VertexBuffer> getVertexBuffer() const = 0;
		virtual Ref<IndexBuffer> getIndexBuffer() const = 0;

		virtual uint32_t getIndexCount() const = 0;

		static Ref<Mesh> create(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);


	};

}
