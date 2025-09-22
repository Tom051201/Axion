#include "axpch.h"
#include "AssetManager.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "AxionEngine/Vendor/tinyobjloader/tiny_obj_loader.h"
#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionEngine/Source/core/YamlHelper.h"
#include "AxionEngine/Source/core/EnumUtils.h"
#include "AxionEngine/Source/project/ProjectManager.h"

namespace Axion {

	void AssetManager::initialize() {
		AX_CORE_LOG_TRACE("AssetManager initialized");
	}

	void AssetManager::shutdown() {
		release<Mesh>();
		release<Skybox>();
		release<Shader>();
	}

	void AssetManager::onEvent(Event& e) {
		// -- RenderingFinished --
		if (e.getEventType() == EventType::RenderingFinished) {
			// -- Load Queue Skybox --
			processLoadQueue<Skybox>([](const std::string& path, const AssetHandle<Skybox>& handle) -> Ref<Skybox> {
				return std::make_shared<Skybox>(path);
			});

			// -- Load Queue Mesh --
			processLoadQueue<Mesh>([](const std::string& path, const AssetHandle<Mesh>& handle) -> Ref<Mesh> {
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
				return Mesh::create(handle, vertices, indices);
			});

			// -- Load Queue Shader --
			processLoadQueue<Shader>([](const std::string& path, const AssetHandle<Shader>& handle) -> Ref<Shader> {
				Ref<Shader> shader = AssetManager::get<Shader>(handle);
				shader->compileFromFile(path);
				return shader;
			});
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
	template<>
	AssetHandle<Mesh> AssetManager::load<Mesh>(const std::string& absolutePath) {
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		if (data["Type"].as<std::string>() != "Mesh") {
			AX_CORE_LOG_ERROR("Loading mesh failed, file is not a mesh asset file");
			return {};
		}

		std::string sourcePath = getAbsolute(data["Source"].as<std::string>());
		UUID uuid = data["UUID"].as<UUID>();
		AssetHandle<Mesh> handle(uuid);

		// -- Return if already registered --
		if (has<Mesh>(handle)) {
			return handle;
		}

		storage<Mesh>().assets[handle] = nullptr;
		storage<Mesh>().loadQueue.push_back({ handle, sourcePath });
		storage<Mesh>().handleToPath[handle] = absolutePath;

		return handle;
	}

	// ----- Skybox Assets -----
	template<>
	AssetHandle<Skybox> AssetManager::load<Skybox>(const std::string& absolutePath) {
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		if (data["Type"].as<std::string>() != "Skybox") {
			AX_CORE_LOG_ERROR("Loading skybox failed, file is not a skybox asset file");
			return {};
		}

		std::string sourcePath = getAbsolute(data["Texture"].as<std::string>());
		UUID uuid = data["UUID"].as<UUID>();
		AssetHandle<Skybox> handle(uuid);

		// -- Return if already registered --
		if (has<Skybox>(handle)) {
			return handle;
		}

		storage<Skybox>().assets[handle] = nullptr;
		storage<Skybox>().loadQueue.push_back({ handle, sourcePath });
		storage<Skybox>().handleToPath[handle] = absolutePath;

		return handle;
	}

	// ----- Shader Assets -----
	template<>
	AssetHandle<Shader> AssetManager::load<Shader>(const std::string& absolutePath) {
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		if (data["Type"].as<std::string>() != "Shader") {
			AX_CORE_LOG_ERROR("Loading shader failed, file is not a shader asset file");
			return {};
		}

		std::string sourcePath = getAbsolute(data["Source"].as<std::string>());
		UUID uuid = data["UUID"].as<UUID>();
		// TODO: also check format
		AssetHandle<Shader> handle(uuid);

		// -- Return if already registered --
		if (has<Shader>(handle)) {
			return handle;
		}

		// -- create shader specification --
		YAML::Node specData = data["Specification"];
		ShaderSpecification spec{};
		spec.name = specData["Name"].as<std::string>();
		spec.colorFormat = EnumUtils::colorFormatFromString(specData["ColorFormat"].as<std::string>());
		spec.depthStencilFormat = EnumUtils::depthStencilFormatFromString(specData["DepthStencilFormat"].as<std::string>());
		spec.depthTest = specData["DepthTest"].as<bool>();
		spec.depthWrite = specData["DepthWrite"].as<bool>();
		spec.depthFunction = EnumUtils::depthCompareFromString(specData["DepthFunction"].as<std::string>());
		spec.stencilEnabled = specData["StencilEnabled"].as<bool>();
		spec.sampleCount = specData["SampleCount"].as<uint32_t>();
		spec.cullMode = EnumUtils::cullModeFromString(specData["CullMode"].as<std::string>());
		spec.topology = EnumUtils::primitiveTopologyFromString(specData["Topology"].as<std::string>());
		YAML::Node layoutData = specData["BufferLayout"];
		if (layoutData && layoutData.IsSequence()) {
			std::vector<BufferElement> elements;
			elements.reserve(layoutData.size());

			for (const auto& elemNode : layoutData) {
				std::string name = elemNode["Name"].as<std::string>();
				ShaderDataType type = EnumUtils::shaderDataTypeFromString(elemNode["Type"].as<std::string>());
				BufferElement elem(name, type);

				if (elemNode["Size"]) { elem.size = elemNode["Size"].as<uint32_t>(); }
				if (elemNode["Offset"]) { elem.offset = elemNode["Offset"].as<uint32_t>(); }

				elements.push_back(elem);
			}

			BufferLayout layout(elements);
			layout.calculateOffsetAndStride();
			spec.vertexLayout = layout;

		}

		Ref<Shader> shader = Shader::create(spec);
		storage<Shader>().assets[handle] = shader;
		storage<Shader>().loadQueue.push_back({ handle, sourcePath });
		storage<Shader>().handleToPath[handle] = absolutePath;

		return handle;
	}
}
