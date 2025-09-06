#include "axpch.h"
#include "AssetManager.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "AxionEngine/Vendor/tinyobjloader/tiny_obj_loader.h"

namespace Axion {

	AssetMap<Mesh> AssetManager::s_meshes;
	AssetMap<Skybox> AssetManager::s_skyboxes;

	void AssetManager::initialize() {
		AX_CORE_LOG_TRACE("AssetManager initialized");
	}

	void AssetManager::release() {
		s_meshes.clear();
		s_skyboxes.clear();
	}

	// ----- Mesh Assets -----

	AssetHandle<Mesh> AssetManager::loadMesh(const std::string& path) {
		tinyobj::ObjReader reader;

		if (!reader.Warning().empty()) AX_CORE_LOG_WARN("OBJ warning: {}", reader.Warning());
		
		if (!reader.ParseFromFile(path)) {
			AX_CORE_LOG_ERROR("Failed to load OBJ file: {}", path);
			if (!reader.Error().empty()) AX_CORE_LOG_ERROR("OBJ error: {}", reader.Error());
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
				} else { vertex.normal = { 0.0f, 0.0f, 0.0f }; }

				if (index.texcoord_index >= 0) {
					vertex.texcoord = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1] // flip V
					};
				} else { vertex.texcoord = { 0.0f, 0.0f }; }

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

		AssetHandle<Mesh> handle(path);
		Ref<Mesh> mesh = Mesh::create(handle, vertices, indices);
		s_meshes[handle] = mesh;

		return handle;
	}
	
	Ref<Mesh> AssetManager::getMesh(const AssetHandle<Mesh>& handle) {
		auto it = s_meshes.find(handle);
		if (it != s_meshes.end()) {
			return it->second;
		}
		else {
			AX_CORE_LOG_WARN("Mesh handle not found: {}", handle);
			return nullptr;
		}
	}

	bool AssetManager::hasMesh(const AssetHandle<Mesh>& handle) {
		auto it = s_meshes.find(handle);
		if (it != s_meshes.end()) { return true; }
		else { return false; }
	}

	// ----- Skybox Assets -----

	AssetHandle<Skybox> AssetManager::loadSkybox(const std::string& path) {
		AssetHandle<Skybox> handle(path);
		Ref<Skybox> skybox = std::make_shared<Skybox>(path);
		s_skyboxes[handle] = skybox;
		return handle;
	}

	Ref<Skybox> AssetManager::getSkybox(const AssetHandle<Skybox>& handle) {
		auto it = s_skyboxes.find(handle);
		if (it != s_skyboxes.end()) {
			return it->second;
		}
		else {
			AX_CORE_LOG_WARN("Skybox handle not found: {}", handle);
			return nullptr;
		}
	}

	bool AssetManager::hasSkybox(const AssetHandle<Skybox>& handle) {
		auto it = s_skyboxes.find(handle);
		if (it != s_skyboxes.end()) { return true; }
		else { return false; }
	}

}
