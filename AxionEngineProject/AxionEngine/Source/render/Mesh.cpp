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

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};

				vertex.position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				if (index.normal_index >= 0) {
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}
				else { vertex.normal = { 0.0f, 0.0f, 0.0f }; }

				if (index.texcoord_index >= 0) {
					vertex.texcoord = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1] // flip V
					};
				}
				else { vertex.texcoord = { 0.0f, 0.0f }; }

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

		Vertex::normalizeVertices(vertices);
		return { vertices, indices };
	}

}
