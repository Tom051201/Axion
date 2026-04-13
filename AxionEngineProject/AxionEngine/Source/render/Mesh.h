#pragma once

#include <filesystem>

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

		virtual ~Mesh() = default;

		virtual void release() = 0;

		virtual void render() const = 0;

		virtual Ref<VertexBuffer> getVertexBuffer() const = 0;
		virtual Ref<IndexBuffer> getIndexBuffer() const = 0;
		virtual const std::vector<Submesh>& getSubmeshes() const = 0;

		virtual uint32_t getIndexCount() const = 0;


		static Ref<Mesh> create(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		static Ref<Mesh> create(const MeshData& meshData);

		static Ref<Mesh> createPBRCube();

		static MeshData loadOBJ(const std::filesystem::path& path);

	private:

		static void normalizeVector(DirectX::XMFLOAT3& v);

	};

}
