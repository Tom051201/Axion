#include "OBJImporter.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "AxionAssetPipeline/Vendor/tinyobjloader/tiny_obj_loader.h"

namespace Axion::AAP {

	MeshData OBJImporter::extractMeshes(const std::filesystem::path& path) {
		tinyobj::ObjReader reader;

		if (!reader.Warning().empty()) {
			AX_CORE_LOG_WARN("OBJ warning: {}", reader.Warning());
		}

		if (!reader.ParseFromFile(path.string())) {
			AX_CORE_LOG_ERROR("Failed to load OBJ file: {}", path.string());
			if (!reader.Error().empty()) {
				AX_CORE_LOG_ERROR("OBJ error: {}", reader.Error());
			}
			return {};
		}

		const auto& attrib = reader.GetAttrib();
		const auto& shapes = reader.GetShapes();

		MeshData meshData;
		std::unordered_map<Vertex, uint32_t> uniqueVertices;

		bool hasNormals = !attrib.normals.empty();

		// -- Load data --
		for (const auto& shape : shapes) {

			Submesh submesh;
			submesh.startIndex = static_cast<uint32_t>(meshData.indices.size());
			submesh.baseVertex = 0;

			submesh.materialIndex = 0;
			if (!shape.mesh.material_ids.empty() && shape.mesh.material_ids[0] >= 0) {
				submesh.materialIndex = static_cast<uint32_t>(shape.mesh.material_ids[0]);
			}

			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};

				// -- Position --
				vertex.position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				// -- Normal --
				if (index.normal_index >= 0) {
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}
				else {
					vertex.normal = { 0.0f, 0.0f, 0.0f };
				}

				// -- TexCoord --
				if (index.texcoord_index >= 0) {
					vertex.texcoord = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1] // flip V
					};
				}
				else {
					vertex.texcoord = { 0.0f, 0.0f };
				}

				// -- Tangent (Init to 0) --
				vertex.tangent = { 0.0f, 0.0f, 0.0f };

				// -- Remove duplicates --
				if (uniqueVertices.count(vertex) == 0) {
					uint32_t newIndex = static_cast<uint32_t>(meshData.vertices.size());
					uniqueVertices[vertex] = newIndex;
					meshData.vertices.push_back(vertex);
					meshData.indices.push_back(newIndex);
				}
				else {
					meshData.indices.push_back(uniqueVertices[vertex]);
				}
			}

			submesh.indexCount = static_cast<uint32_t>(meshData.indices.size()) - submesh.startIndex;
			if (submesh.indexCount > 0) {
				meshData.submeshes.push_back(submesh);
			}

		}

		// -- Normalize Mesh --
		Vertex::normalizeVertices(meshData.vertices);

		// -- Calculate Normals if missing --
		if (!hasNormals) {
			AX_CORE_LOG_WARN("OBJ file missing normals, recalculating them...");
			for (size_t i = 0; i < meshData.indices.size(); i += 3) {
				Vertex& v0 = meshData.vertices[meshData.indices[i]];
				Vertex& v1 = meshData.vertices[meshData.indices[i + 1]];
				Vertex& v2 = meshData.vertices[meshData.indices[i + 2]];

				float ax = v1.position.x - v0.position.x;
				float ay = v1.position.y - v0.position.y;
				float az = v1.position.z - v0.position.z;

				float bx = v2.position.x - v0.position.x;
				float by = v2.position.y - v0.position.y;
				float bz = v2.position.z - v0.position.z;

				float nx = ay * bz - az * by;
				float ny = az * bx - ax * bz;
				float nz = ax * by - ay * bx;

				v0.normal.x += nx; v0.normal.y += ny; v0.normal.z += nz;
				v1.normal.x += nx; v1.normal.y += ny; v1.normal.z += nz;
				v2.normal.x += nx; v2.normal.y += ny; v2.normal.z += nz;
			}

			for (auto& v : meshData.vertices) {
				normalizeVector(v.normal);
			}

		}

		// -- Calculate Tangents --
		for (size_t i = 0; i < meshData.indices.size(); i += 3) {
			Vertex& v0 = meshData.vertices[meshData.indices[i]];
			Vertex& v1 = meshData.vertices[meshData.indices[i + 1]];
			Vertex& v2 = meshData.vertices[meshData.indices[i + 2]];

			// -- Edge Vectors --
			float e1x = v1.position.x - v0.position.x;
			float e1y = v1.position.y - v0.position.y;
			float e1z = v1.position.z - v0.position.z;

			float e2x = v2.position.x - v0.position.x;
			float e2y = v2.position.y - v0.position.y;
			float e2z = v2.position.z - v0.position.z;

			// -- Delta UVs --
			float du1 = v1.texcoord.x - v0.texcoord.x;
			float dv1 = v1.texcoord.y - v0.texcoord.y;
			float du2 = v2.texcoord.x - v0.texcoord.x;
			float dv2 = v2.texcoord.y - v0.texcoord.y;

			// -- Determinant --
			float f = 1.0f / (du1 * dv2 - du2 * dv1);
			if (std::isinf(f) || std::isnan(f)) f = 0.0f;

			float tx = f * (dv2 * e1x - dv1 * e2x);
			float ty = f * (dv2 * e1y - dv1 * e2y);
			float tz = f * (dv2 * e1z - dv1 * e2z);

			// -- Accumulate Tangents --
			v0.tangent.x += tx; v0.tangent.y += ty; v0.tangent.z += tz;
			v1.tangent.x += tx; v1.tangent.y += ty; v1.tangent.z += tz;
			v2.tangent.x += tx; v2.tangent.y += ty; v2.tangent.z += tz;
		}

		// Normalize Tangents and orthogonalize (Gram-Schmidt)
		for (auto& v : meshData.vertices) {
			normalizeVector(v.tangent);

			// t = normalize(t - n * dot(n, t));
			float dot = v.normal.x * v.tangent.x + v.normal.y * v.tangent.y + v.normal.z * v.tangent.z;
			v.tangent.x -= v.normal.x * dot;
			v.tangent.y -= v.normal.y * dot;
			v.tangent.z -= v.normal.z * dot;
			normalizeVector(v.tangent);
		}

		return meshData;
	}

	void OBJImporter::normalizeVector(DirectX::XMFLOAT3& v) {
		float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
		if (length > 0.0f) {
			float inv = 1.0f / length;
			v.x *= inv;
			v.y *= inv;
			v.z *= inv;
		}
	}

}
