#include "axpch.h"
#include "Mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "AxionEngine/Vendor/tinyobjloader/tiny_obj_loader.h"
#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionEngine/Source/render/Renderer.h"

#include "AxionEngine/Platform/directx/D12Mesh.h"
#include "AxionEngine/Platform/opengl/OpenGL3Mesh.h"

namespace Axion {

	Ref<Mesh> Mesh::create(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {

		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); break; }
			case RendererAPI::DirectX12: { return std::make_shared<D12Mesh>(vertices, indices); }
			case RendererAPI::OpenGL3: { return std::make_shared<OpenGL3Mesh>(vertices, indices); }

		}
		return nullptr;

	}



	// ----- Loading OBJ Files -----
	MeshData Mesh::loadOBJ(const std::string& path) {
		tinyobj::ObjReader reader;

		if (!reader.Warning().empty()) {
			AX_CORE_LOG_WARN("OBJ warning: {}", reader.Warning());
		}

		if (!reader.ParseFromFile(path)) {
			AX_CORE_LOG_ERROR("Failed to load OBJ file: {}", path);
			if (!reader.Error().empty()) {
				AX_CORE_LOG_ERROR("OBJ error: {}", reader.Error());
			}
			return {};
		}

		const auto& attrib = reader.GetAttrib();
		const auto& shapes = reader.GetShapes();

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::unordered_map<Vertex, uint32_t> uniqueVertices;

		bool hasNormals = !attrib.normals.empty();

		// -- Load data --
		for (const auto& shape : shapes) {
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
					uint32_t newIndex = static_cast<uint32_t>(vertices.size());
					uniqueVertices[vertex] = newIndex;
					vertices.push_back(vertex);
					indices.push_back(newIndex);
				}
				else {
					indices.push_back(uniqueVertices[vertex]);
				}
			}
		}

		// -- Normalize Mesh --
		Vertex::normalizeVertices(vertices);

		// -- Calculate Normals if missing --
		if (!hasNormals) {
			AX_CORE_LOG_WARN("OBJ file missing normals, recalculating them...");
			for (size_t i = 0; i < indices.size(); i += 3) {
				Vertex& v0 = vertices[indices[i]];
				Vertex& v1 = vertices[indices[i + 1]];
				Vertex& v2 = vertices[indices[i + 2]];

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

			for (auto& v : vertices) {
				normalizeVector(v.normal);
			}

		}

		// -- Calculate Tangents --
		for (size_t i = 0; i < indices.size(); i += 3) {
			Vertex& v0 = vertices[indices[i]];
			Vertex& v1 = vertices[indices[i + 1]];
			Vertex& v2 = vertices[indices[i + 2]];

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
		for (auto& v : vertices) {
			normalizeVector(v.tangent);

			// t = normalize(t - n * dot(n, t));
			float dot = v.normal.x * v.tangent.x + v.normal.y * v.tangent.y + v.normal.z * v.tangent.z;
			v.tangent.x -= v.normal.x * dot;
			v.tangent.y -= v.normal.y * dot;
			v.tangent.z -= v.normal.z * dot;
			normalizeVector(v.tangent);
		}

		return { vertices, indices };
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



	// ----- Helper Function -----
	void Mesh::normalizeVector(DirectX::XMFLOAT3& v) {
		float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
		if (length > 0.0f) {
			float inv = 1.0f / length;
			v.x *= inv;
			v.y *= inv;
			v.z *= inv;
		}
	}

}
