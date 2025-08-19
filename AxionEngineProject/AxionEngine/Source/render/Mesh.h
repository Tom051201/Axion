#pragma once

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/render/Buffers.h"
#include "AxionEngine/Source/core/AssetHandle.h"

namespace Axion {

	class Mesh {
	public:

		virtual ~Mesh() = default;

		virtual void release() = 0;

		virtual void render() const = 0;

		virtual Ref<VertexBuffer> getVertexBuffer() const = 0;
		virtual Ref<IndexBuffer> getIndexBuffer() const = 0;

		virtual uint32_t getIndexCount() const = 0;

		virtual const AssetHandle<Mesh>& getHandle() const = 0;


		static Ref<Mesh> create(const AssetHandle<Mesh>& handle, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);


	};

}
