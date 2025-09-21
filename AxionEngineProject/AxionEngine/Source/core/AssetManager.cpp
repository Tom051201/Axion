#include "axpch.h"
#include "AssetManager.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "AxionEngine/Vendor/tinyobjloader/tiny_obj_loader.h"
#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionEngine/Source/core/YamlHelper.h"
#include "AxionEngine/Source/project/ProjectManager.h"

namespace Axion {

	AssetMap<Mesh> AssetManager::s_meshes;
	HandleToPathMap<Mesh> AssetManager::s_meshHandleToPath;

	AssetMap<Skybox> AssetManager::s_skyboxes;
	HandleToPathMap<Skybox> AssetManager::s_skyboxHandleToPath;
	LoadQueue<Skybox> AssetManager::s_skyboxLoadQueue;

	void AssetManager::initialize() {
		AX_CORE_LOG_TRACE("AssetManager initialized");
	}

	void AssetManager::release() {
		s_meshes.clear();
		s_skyboxes.clear();
	}

	void AssetManager::onEvent(Event& e) {
		// -- RenderingFinished --
		if (e.getEventType() == EventType::RenderingFinished) {
			// -- Load Queues Skybox --
			for (auto& sky : s_skyboxLoadQueue) {
				Ref<Skybox> skybox = std::make_shared<Skybox>(sky.second);
				s_skyboxes[sky.first] = skybox;
				AX_CORE_LOG_INFO("Skybox loaded: {}", sky.first.uuid.toString());
			}
			s_skyboxLoadQueue.clear();
		}
	}

	std::string AssetManager::getRelativeToAssets(const std::string& absolutePath) {
		if (ProjectManager::hasProject()) {
			std::filesystem::path absPath(absolutePath);
			std::filesystem::path assetsDir = ProjectManager::getProject()->getAssetsPath();
			std::filesystem::path relPath = std::filesystem::relative(absPath, assetsDir);
			return relPath.string();
		}
		else {
			AX_CORE_LOG_WARN("Unable converting absolute path to relative assets path: no project loaded");
			return {};
		}
	}

	std::string AssetManager::getAbsolute(const std::string& relativePath) {
		if (ProjectManager::hasProject()) {
			std::string absPath = ProjectManager::getProject()->getAssetsPath() + "\\" + relativePath;
			if (std::filesystem::exists(absPath)) {
				return absPath;
			}
			else {
				AX_CORE_LOG_WARN("Unable converting relative path to absolute assets path: path does not exist");
				return {};
			}
		}
		else {
			AX_CORE_LOG_WARN("Unable converting relative path to absolute assets path: no project loaded");
			return {};
		}
	}

	// ----- Mesh Assets -----

	AssetHandle<Mesh> AssetManager::loadMesh(const std::string& absolutePath) {
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		if (data["Type"].as<std::string>() != "Mesh") {
			AX_CORE_LOG_ERROR("Loading mesh failed, file is not a mesh asset file");
			return {};
		}

		std::string sourcePath = getAbsolute(data["Source"].as<std::string>());
		UUID uuid = data["UUID"].as<UUID>();

		tinyobj::ObjReader reader;

		if (!reader.Warning().empty()) AX_CORE_LOG_WARN("OBJ warning: {}", reader.Warning());

		if (!reader.ParseFromFile(sourcePath)) {
			AX_CORE_LOG_ERROR("Failed to load OBJ file: {}", sourcePath);
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

		AssetHandle<Mesh> handle(uuid);
		
		// -- Only load if not already loaded --
		if (!hasMesh(handle)) {
			Ref<Mesh> mesh = Mesh::create(handle, vertices, indices);
			s_meshes[handle] = mesh;
			s_meshHandleToPath[handle] = absolutePath;
			AX_CORE_LOG_INFO("Mesh loaded: {}", uuid.toString());
		}

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

	const std::string& AssetManager::getMeshAssetFilePath(const AssetHandle<Mesh>& handle) {
		return s_meshHandleToPath[handle];
	}

	// ----- Skybox Assets -----

	AssetHandle<Skybox> AssetManager::loadSkybox(const std::string& absolutePath) {
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		if (data["Type"].as<std::string>() != "Skybox") {
			AX_CORE_LOG_ERROR("Loading skybox failed, file is not a skybox asset file");
			return {};
		}

		std::string sourcePath = getAbsolute(data["Texture"].as<std::string>());
		UUID uuid = data["UUID"].as<UUID>();

		AssetHandle<Skybox> handle(uuid);
		
		// -- Only load if not already loaded --
		if (!hasSkybox(handle)) {
			s_skyboxes[handle] = nullptr;
			s_skyboxLoadQueue.push_back({ handle, sourcePath });
			s_skyboxHandleToPath[handle] = absolutePath;
		}
		

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

	const std::string& AssetManager::getSkyboxAssetFilePath(const AssetHandle<Skybox>& handle) {
		return s_skyboxHandleToPath[handle];
	}

}
