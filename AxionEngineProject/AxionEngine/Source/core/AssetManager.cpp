#include "axpch.h"
#include "AssetManager.h"

#include "AxionEngine/Source/core/YamlHelper.h"
#include "AxionEngine/Source/core/EnumUtils.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/core/BinaryHeaders.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/scene/Skybox.h"
#include "AxionEngine/Source/scene/Prefab.h"
#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Shader.h"
#include "AxionEngine/Source/render/Material.h"
#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/audio/AudioClip.h"
#include "AxionEngine/Source/physics/PhysicsMaterial.h"

namespace Axion {

	void AssetManager::initialize() {
		AX_CORE_LOG_TRACE("AssetManager initialized");
	}

	void AssetManager::shutdown() {
		release<Mesh>();
		release<TextureCube>();
		release<Skybox>();
		release<Shader>();
		release<Material>();
		release<AudioClip>();
		release<Texture2D>();
		release<Pipeline>();
		release<PhysicsMaterial>();
		release<Prefab>();
		AX_CORE_LOG_INFO("AssetManager shutdown");
	}

	void AssetManager::onEvent(Event& e) {
		// -- RenderingFinished --
		if (e.getEventType() == EventType::RenderingFinished) {

			processLoadQueue<Skybox>();
			processLoadQueue<Mesh>();
			processLoadQueue<Shader>();
			processLoadQueue<Pipeline>();
			processLoadQueue<Texture2D>();
			// TODO: add all queues

		}
	}

	std::string AssetManager::getRelativeToAssets(const std::string& absolutePath) {
		if (!ProjectManager::hasProject()) {
			AX_CORE_LOG_WARN("Unable converting absolute path to relative assets path: no project loaded");
			return {};
		}

		std::filesystem::path absPath(absolutePath);
		std::filesystem::path assetsDir = ProjectManager::getProject()->getAssetsPath();

		try {
			std::filesystem::path relPath = std::filesystem::relative(absPath, assetsDir);
			return relPath.string();
		} catch (const std::exception& e) {
			AX_CORE_LOG_WARN("Failed to convert absolute path to relative: {}", e.what());
			(void)e;
			return {};
		}
	}

	std::filesystem::path AssetManager::getRelativeToAssets(const std::filesystem::path& absolutePath) {
		if (!ProjectManager::hasProject()) {
			AX_CORE_LOG_WARN("Unable converting absolute path to relative assets path: no project loaded");
			return {};
		}

		std::filesystem::path assetsDir = ProjectManager::getProject()->getAssetsPath();

		try {
			std::filesystem::path relPath = std::filesystem::relative(absolutePath, assetsDir);
			return relPath;
		}
		catch (const std::exception& e) {
			AX_CORE_LOG_WARN("Failed to convert absolute path to relative: {}", e.what());
			(void)e;
			return {};
		}
	}

	std::string AssetManager::getAbsolute(const std::string& relativePath) {
		if (!ProjectManager::hasProject()) {
			AX_CORE_LOG_WARN("Unable converting relative path to absolute assets path: no project loaded");
			return {};
		}

		std::filesystem::path absPath = ProjectManager::getProject()->getAssetsPath();
		absPath /= relativePath;
		if (!std::filesystem::exists(absPath)) {
			AX_CORE_LOG_WARN("Unable converting relative path to absolute assets path: path does not exist");
			return {};
		}

		return absPath.string();
	}

	UUID AssetManager::getAssetUUID(const std::string& absolutePath) {
		try {
			std::ifstream stream(absolutePath);
			if (!stream.is_open()) {
				return {};
			}

			YAML::Node data = YAML::Load(stream);
			if (data["UUID"]) {
				return data["UUID"].as<UUID>();
			}
		}
		catch (const std::exception& e) {
			AX_CORE_LOG_WARN("Failed to read UUID from asset file '{0}': {1}", absolutePath, e.what());
		}

		return {};
	}

	// ----- Mesh Assets -----
	template<>
	AssetHandle<Mesh> AssetManager::load<Mesh>(UUID handle) {
		if (has<Mesh>(handle)) return handle;

		auto registry = ProjectManager::getProject()->getAssetRegistry();
		if (!registry->contains(handle)) {
			AX_CORE_LOG_ERROR("Mesh UUID not found in AssetRegistry!");
			return {};
		}

		std::string absolutePath = getAbsolute(registry->get(handle).filePath.string());

		// -- Load Binary if in Runtime Mode --
		if (ProjectManager::isRuntime()) {
			storage<Mesh>().assets[handle] = nullptr;
			storage<Mesh>().loadQueue.push_back({ handle,
				[absolutePath]() {
					std::ifstream in(absolutePath, std::ios::in | std::ios::binary);

					BinaryAssetHeader header;
					in.read(reinterpret_cast<char*>(&header), sizeof(BinaryAssetHeader));

					uint32_t vertexCount, indexCount;
					in.read(reinterpret_cast<char*>(&vertexCount), sizeof(uint32_t));
					in.read(reinterpret_cast<char*>(&indexCount), sizeof(uint32_t));

					std::vector<Vertex> vertices(vertexCount);
					std::vector<uint32_t> indices(indexCount);

					in.read(reinterpret_cast<char*>(vertices.data()), vertexCount * sizeof(Vertex));
					in.read(reinterpret_cast<char*>(indices.data()), indexCount * sizeof(uint32_t));

					return Mesh::create(vertices, indices);
				}
			});
			storage<Mesh>().handleToPath[handle] = absolutePath;
			return handle;
		}

		// -- Load YAML if not in Runtime Mode --
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
		if (version == ASSET_VERSION_MESH) {
			std::string sourcePath = getAbsolute(data["Source"].as<std::string>());

			storage<Mesh>().assets[handle] = nullptr;
			storage<Mesh>().loadQueue.push_back({ handle,
				[sourcePath]() {
					auto meshData = Mesh::loadOBJ(sourcePath);
					return Mesh::create(meshData.vertices, meshData.indices);
				}
			});
			storage<Mesh>().handleToPath[handle] = absolutePath;

			return handle;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported Mesh Version: {} in file {}", version, absolutePath);
			return {};
		}

	}

	// ----- TextureCube Assets -----
	template<>
	AssetHandle<TextureCube> AssetManager::load<TextureCube>(UUID handle) {
		if (has<TextureCube>(handle)) return handle;

		auto registry = ProjectManager::getProject()->getAssetRegistry();
		if (!registry->contains(handle)) {
			AX_CORE_LOG_ERROR("TextureCube UUID not found in AssetRegistry!");
			return {};
		}

		std::string absolutePath = getAbsolute(registry->get(handle).filePath.string());

		// -- Load Binary if in Runtime Mode --
		if (ProjectManager::isRuntime()) {
			storage<TextureCube>().assets[handle] = nullptr;
			storage<TextureCube>().loadQueue.push_back({ handle,
				[absolutePath]() {
					std::ifstream in(absolutePath, std::ios::in | std::ios::binary);

					BinaryAssetHeader header;
					in.read(reinterpret_cast<char*>(&header), sizeof(BinaryAssetHeader));

					uint64_t dataSize;
					in.read(reinterpret_cast<char*>(&dataSize), sizeof(uint64_t));

					std::vector<uint8_t> imageData(dataSize);
					in.read(reinterpret_cast<char*>(imageData.data()), dataSize);

					return TextureCube::create(imageData.data(), imageData.size());
				}
			});
			storage<TextureCube>().handleToPath[handle] = absolutePath;
			return handle;
		}

		// -- Load YAML if not in Runtime Mode --
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
		if (version == ASSET_VERSION_TEXTURE_CUBE) {
			std::string sourcePath = getAbsolute(data["Source"].as<std::string>());
			UUID uuid = data["UUID"].as<UUID>();

			storage<TextureCube>().assets[handle] = nullptr;
			storage<TextureCube>().loadQueue.push_back({ handle,
				[sourcePath, handle]() {
					return TextureCube::create(sourcePath);
				}
			});
			storage<TextureCube>().handleToPath[handle] = absolutePath;

			return handle;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported TextureCube Version: {} in file {}", version, absolutePath);
			return {};
		}

	}

	// ----- Skybox Assets -----
	template<>
	AssetHandle<Skybox> AssetManager::load<Skybox>(UUID handle) {
		if (has<Skybox>(handle)) return handle;

		auto registry = ProjectManager::getProject()->getAssetRegistry();
		if (!registry->contains(handle)) {
			AX_CORE_LOG_ERROR("Skybox UUID not found in AssetRegistry!");
			return {};
		}

		std::string absolutePath = getAbsolute(registry->get(handle).filePath.string());

		// -- Load Binary if in Runtime Mode --
		if (ProjectManager::isRuntime()) {
			storage<Skybox>().assets[handle] = nullptr;
			storage<Skybox>().loadQueue.push_back({ handle,
				[absolutePath]() {
					std::ifstream in(absolutePath, std::ios::in | std::ios::binary);

					SkyboxBinaryHeader header;
					in.read(reinterpret_cast<char*>(&header), sizeof(SkyboxBinaryHeader));

					AssetHandle<TextureCube> texHandle = AssetManager::load<TextureCube>(header.textureCubeUUID);
					AssetHandle<Pipeline> pipeHandle = AssetManager::load<Pipeline>(header.pipelineUUID);

					return std::make_shared<Skybox>(texHandle, pipeHandle);
				}
				});
			storage<Skybox>().handleToPath[handle] = absolutePath;
			return handle;
		}

		// -- Load YAML if not in Runtime Mode --
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
		if (version == ASSET_VERSION_SKYBOX) {

			UUID texUUID = data["TextureCube"].as<UUID>();
			UUID pipeUUID = data["Pipeline"].as<UUID>();

			storage<Skybox>().assets[handle] = nullptr;
			storage<Skybox>().loadQueue.push_back({ handle,
				[texUUID, pipeUUID]() {
					AssetHandle<TextureCube> texHandle = AssetManager::load<TextureCube>(texUUID);
					AssetHandle<Pipeline> pipeHandle = AssetManager::load<Pipeline>(pipeUUID);
					return std::make_shared<Skybox>(texHandle, pipeHandle);
				}
			});
			storage<Skybox>().handleToPath[handle] = absolutePath;

			return handle;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported Skybox Version: {} in file {}", version, absolutePath);
			return {};
		}
	}

	// ----- Shader Assets -----
	template<>
	AssetHandle<Shader> AssetManager::load<Shader>(UUID handle) {
		if (has<Shader>(handle)) return handle;

		auto registry = ProjectManager::getProject()->getAssetRegistry();
		if (!registry->contains(handle)) {
			AX_CORE_LOG_ERROR("Shader UUID not found in AssetRegistry!");
			return {};
		}

		std::string absolutePath = getAbsolute(registry->get(handle).filePath.string());

		// -- Load Binary if in Runtime Mode --
		if (ProjectManager::isRuntime()) {
			storage<Shader>().assets[handle] = nullptr;
			storage<Shader>().loadQueue.push_back({ handle,
				[absolutePath]() {
					std::ifstream in(absolutePath, std::ios::in | std::ios::binary);

					ShaderBinaryHeader header;
					in.read(reinterpret_cast<char*>(&header), sizeof(ShaderBinaryHeader));

					ShaderSpecification spec = {};
					spec.name = "RuntimeShader_" + header.assetHeader.uuid.toString();
					spec.batchTextures = header.batchTextures;

					Ref<Shader> shader = Shader::create(spec);

					std::vector<uint8_t> vsData(header.vsSize);
					in.read(reinterpret_cast<char*>(vsData.data()), header.vsSize);
					std::vector<uint8_t> psData(header.psSize);
					in.read(reinterpret_cast<char*>(psData.data()), header.psSize);

					shader->loadFromBytecode(vsData.data(), header.vsSize, psData.data(), header.psSize);

					return shader;
				}
			});
			storage<Shader>().handleToPath[handle] = absolutePath;
			return handle;
		}

		// -- Load YAML if not in Runtime Mode --
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
		if (version == ASSET_VERSION_SHADER) {
			std::string sourcePath = getAbsolute(data["Source"].as<std::string>());
			UUID uuid = data["UUID"].as<UUID>();

			// -- Check format --
			std::string format = data["Format"].as<std::string>();
			RendererAPI api = Renderer::getAPI();

			bool formatMatches =
				(format == ".hlsl" && api == RendererAPI::DirectX12) ||
				(format == ".glsl" && api == RendererAPI::OpenGL3);

			// -- Check extension --
			std::filesystem::path srcPath(sourcePath);
			std::string ext = srcPath.extension().string();

			bool extensionMatches =
				(api == RendererAPI::DirectX12 && ext == ".hlsl") ||
				(api == RendererAPI::OpenGL3 && ext == ".glsl");

			if (!formatMatches) {
				if (extensionMatches) {
					AX_CORE_LOG_WARN("Shader format '{}' in '{}' does not match current RendererAPI, but file extension '{}' is valid. Attempting to load anyway.",
						format, absolutePath, ext);
				} else {
					AX_CORE_LOG_ERROR("Shader format '{}' in '{}' is not supported by current RendererAPI and file extension '{}' does not match expected format",
						format, absolutePath, ext);
					return {};
				}
			}


			// -- create shader specification --
			YAML::Node specData = data["Specification"];
			ShaderSpecification spec = {};
			spec.name = data["Name"].as<std::string>();
			if (specData["BatchTextures"]) {
				spec.batchTextures = specData["BatchTextures"].as<uint32_t>();
			}

			Ref<Shader> shader = Shader::create(spec, sourcePath);
			storage<Shader>().assets[handle] = shader;
			storage<Shader>().loadQueue.push_back({ handle, 
				[sourcePath, handle]() {
					Ref<Shader> shader = AssetManager::get<Shader>(handle);
					shader->compileFromFile(sourcePath);
					return shader;
				}
			});
			storage<Shader>().handleToPath[handle] = absolutePath;

			return handle;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported Shader Version: {} in file {}", version, absolutePath);
			return {};
		}

	}

	// ----- Pipeline Assets -----
	template<>
	AssetHandle<Pipeline> AssetManager::load<Pipeline>(UUID handle) {
		if (has<Pipeline>(handle)) return handle;

		auto registry = ProjectManager::getProject()->getAssetRegistry();
		if (!registry->contains(handle)) {
			AX_CORE_LOG_ERROR("Pipeline UUID not found in AssetRegistry!");
			return {};
		}

		std::string absolutePath = getAbsolute(registry->get(handle).filePath.string());

		// -- Load Binary if in Runtime Mode --
		if (ProjectManager::isRuntime()) {
			storage<Pipeline>().assets[handle] = nullptr;
			storage<Pipeline>().loadQueue.push_back({ handle,
				[absolutePath]() {
					std::ifstream in(absolutePath, std::ios::in | std::ios::binary);

					PipelineBinaryHeader header;
					in.read(reinterpret_cast<char*>(&header), sizeof(PipelineBinaryHeader));

					PipelineSpecification spec = {};
					spec.colorFormat = static_cast<ColorFormat>(header.colorFormat);
					spec.depthStencilFormat = static_cast<DepthStencilFormat>(header.depthStencilFormat);
					spec.depthTest = header.depthTest != 0;
					spec.depthWrite = header.depthWrite != 0;
					spec.depthFunction = static_cast<DepthCompare>(header.depthFunction);
					spec.stencilEnabled = header.stencilEnabled != 0;
					spec.sampleCount = header.sampleCount;
					spec.cullMode = static_cast<CullMode>(header.cullMode);
					spec.topology = static_cast<PrimitiveTopology>(header.topology);
					spec.numRenderTargets = header.numRenderTargets;

					std::vector<BufferElement> elements;
					elements.reserve(header.bufferElementCount);

					for (uint32_t i = 0; i < header.bufferElementCount; i++) {
						uint32_t nameLen;
						in.read(reinterpret_cast<char*>(&nameLen), sizeof(uint32_t));
						std::string semanticName(nameLen, '\0');
						in.read(&semanticName[0], nameLen);

						uint32_t typeInt;
						uint32_t size, offset;
						uint8_t instanced;

						in.read(reinterpret_cast<char*>(&typeInt), sizeof(uint32_t));
						in.read(reinterpret_cast<char*>(&size), sizeof(uint32_t));
						in.read(reinterpret_cast<char*>(&offset), sizeof(uint32_t));
						in.read(reinterpret_cast<char*>(&instanced), sizeof(uint8_t));

						BufferElement elem(semanticName, static_cast<ShaderDataType>(typeInt));
						elem.size = size;
						elem.offset = offset;
						elem.instanced = instanced != 0;
						elements.push_back(elem);
					}

					BufferLayout layout(elements);
					layout.calculateOffsetAndStride();
					spec.vertexLayout = layout;

					AssetHandle<Shader> shaderHandle = AssetManager::load<Shader>(header.shaderUUID);
					spec.shader = AssetManager::get<Shader>(shaderHandle);

					AX_CORE_ASSERT(spec.shader, "Shader must be valid before creating pipeline!");
					return Pipeline::create(spec);
				}
				});
			storage<Pipeline>().handleToPath[handle] = absolutePath;
			return handle;
		}

		// -- Load YAML if not in Runtime Mode --
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
		if (version == ASSET_VERSION_PIPELINE) {
			YAML::Node specData = data["Specification"];

			UUID shaderUUID = specData["Shader"].as<UUID>();
			AssetHandle<Shader> shaderHandle = AssetManager::load<Shader>(shaderUUID);

			PipelineSpecification spec = {};
			spec.numRenderTargets = specData["NumRenderTargets"].as<uint32_t>();
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
					if (elemNode["Instanced"]) { elem.instanced = elemNode["Instanced"].as<bool>(); }

					elements.push_back(elem);
				}

				BufferLayout layout(elements);
				layout.calculateOffsetAndStride();
				spec.vertexLayout = layout;
			}

			storage<Pipeline>().assets[handle] = nullptr;
			storage<Pipeline>().loadQueue.push_back({ handle,
				[shaderHandle, spec]() mutable {
					spec.shader = AssetManager::get<Shader>(shaderHandle);
					AX_CORE_ASSERT(spec.shader, "Shader must be valid before creating pipeline!");
					return Pipeline::create(spec);
				}
				});
			storage<Pipeline>().handleToPath[handle] = absolutePath;

			return handle;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported Pipeline Version: {} in file {}", version, absolutePath);
			return {};
		}

	}

	// ----- Material Assets -----
	template<>
	AssetHandle<Material> AssetManager::load<Material>(UUID handle) {
		if (has<Material>(handle)) return handle;

		auto registry = ProjectManager::getProject()->getAssetRegistry();
		if (!registry->contains(handle)) {
			AX_CORE_LOG_ERROR("Material UUID not found in AssetRegistry!");
			return {};
		}

		std::string absolutePath = getAbsolute(registry->get(handle).filePath.string());
		
		// -- Load Binary if in Runtime Mode --
		if (ProjectManager::isRuntime()) {
			storage<Material>().assets[handle] = nullptr;
			storage<Material>().loadQueue.push_back({ handle,
				[absolutePath]() {
					std::ifstream in(absolutePath, std::ios::in | std::ios::binary);

					MaterialBinaryHeader header;
					in.read(reinterpret_cast<char*>(&header), sizeof(MaterialBinaryHeader));

					AssetHandle<Pipeline> pipelineHandle = AssetManager::load<Pipeline>(header.pipelineUUID);
					Ref<Material> material = Material::create("", pipelineHandle, header.properties);

					for (uint32_t i = 0; i < header.textureCount; i++) {
						uint32_t slotInt;
						in.read(reinterpret_cast<char*>(&slotInt), sizeof(uint32_t));
						TextureSlot slot = static_cast<TextureSlot>(slotInt);

						UUID textureUUID;
						in.read(reinterpret_cast<char*>(&textureUUID), sizeof(UUID));

						AssetHandle<Texture2D> textureHandle = AssetManager::load<Texture2D>(textureUUID);
						material->setTexture(slot, textureHandle);
					}

					return material;
				}
			});
			storage<Material>().handleToPath[handle] = absolutePath;
			return handle;
		}

		// -- Load YAML if not in Runtime Mode --
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
		if (version == ASSET_VERSION_MATERIAL) {
			std::string name = data["Name"].as<std::string>();

			MaterialProperties prop;
			prop.albedoColor = data["AlbedoColor"].as<Vec4>();
			prop.metalness = data["Metalness"].as<float>();
			prop.roughness = data["Roughness"].as<float>();
			prop.emissionStrength = data["Emission"].as<float>();
			prop.tiling = data["Tiling"].as<float>();
			prop.useNormalMap = data["UseNormalMap"].as<float>();
			prop.useMetalnessMap = data["UseMetalnessMap"].as<float>();
			prop.useRoughnessMap = data["UseRoughnessMap"].as<float>();
			prop.useOcclusionMap = data["UseOcclusionMap"].as<float>();

			UUID pipelineUUID = data["Pipeline"].as<UUID>();
			AssetHandle<Pipeline> pipelineHandle = AssetManager::load<Pipeline>(pipelineUUID);
			Ref<Material> material = Material::create(name, pipelineHandle, prop);

			if (data["Textures"]) {
				auto textures = data["Textures"];

				if (textures["Albedo"] && textures["Albedo"].as<UUID>().isValid()) {
					AssetHandle<Texture2D> handle = AssetManager::load<Texture2D>(textures["Albedo"].as<UUID>());
					material->setTexture(TextureSlot::Albedo, handle);
				}

				if (textures["Normal"] && textures["Normal"].as<UUID>().isValid()) {
					AssetHandle<Texture2D> handle = AssetManager::load<Texture2D>(textures["Normal"].as<UUID>());
					material->setTexture(TextureSlot::Normal, handle);
				}

				if (textures["Metalness"] && textures["Metalness"].as<UUID>().isValid()) {
					AssetHandle<Texture2D> handle = AssetManager::load<Texture2D>(textures["Metalness"].as<UUID>());
					material->setTexture(TextureSlot::Metalness, handle);
				}

				if (textures["Roughness"] && textures["Roughness"].as<UUID>().isValid()) {
					AssetHandle<Texture2D> handle = AssetManager::load<Texture2D>(textures["Roughness"].as<UUID>());
					material->setTexture(TextureSlot::Roughness, handle);
				}

				if (textures["Occlusion"] && textures["Occlusion"].as<UUID>().isValid()) {
					AssetHandle<Texture2D> handle = AssetManager::load<Texture2D>(textures["Occlusion"].as<UUID>());
					material->setTexture(TextureSlot::Occlusion, handle);
				}

				if (textures["Emissive"] && textures["Emissive"].as<UUID>().isValid()) {
					AssetHandle<Texture2D> handle = AssetManager::load<Texture2D>(textures["Emissive"].as<UUID>());
					material->setTexture(TextureSlot::Emissive, handle);
				}

			}

			storage<Material>().assets[handle] = material;
			storage<Material>().handleToPath[handle] = absolutePath;

			return handle;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported Material Version: {} in file {}", version, absolutePath);
			return {};
		}

	}

	template<>
	void AssetManager::reload<Material>(const AssetHandle<Material>& handle) {
		if (!has<Material>(handle)) {
			AX_CORE_LOG_WARN("Cannot reload material: Handle not found in registry");
			return;
		}

		std::string absolutePath = getAssetFilePath(handle);
		std::ifstream stream(absolutePath);
		if (!stream.is_open()) {
			AX_CORE_LOG_ERROR("Failed to open material file for reloading: {}", absolutePath);
			return;
		}

		YAML::Node data = YAML::Load(stream);
		if (data["Type"].as<std::string>() != "Material") {
			AX_CORE_LOG_ERROR("Reloading material failed, file is not a material asset file");
			return;
		}

		Ref<Material> material = get<Material>(handle);

		MaterialProperties prop;
		prop.albedoColor = data["AlbedoColor"].as<Vec4>();
		prop.metalness = data["Metalness"].as<float>();
		prop.roughness = data["Roughness"].as<float>();
		prop.emissionStrength = data["Emission"].as<float>();
		prop.tiling = data["Tiling"].as<float>();
		prop.useNormalMap = data["UseNormalMap"].as<float>();
		prop.useMetalnessMap = data["UseMetalnessMap"].as<float>();
		prop.useRoughnessMap = data["UseRoughnessMap"].as<float>();
		prop.useOcclusionMap = data["UseOcclusionMap"].as<float>();

		material->setProperties(prop);
		material->clearTextures();

		if (data["Textures"]) {
			auto textures = data["Textures"];

			if (textures["Albedo"] && textures["Albedo"].as<UUID>().isValid()) {
				AssetHandle<Texture2D> texHandle = AssetManager::load<Texture2D>(textures["Albedo"].as<UUID>());
				material->setTexture(TextureSlot::Albedo, texHandle);
			}

			if (textures["Normal"] && textures["Normal"].as<UUID>().isValid()) {
				AssetHandle<Texture2D> texHandle = AssetManager::load<Texture2D>(textures["Normal"].as<UUID>());
				material->setTexture(TextureSlot::Normal, texHandle);
			}

			if (textures["Metalness"] && textures["Metalness"].as<UUID>().isValid()) {
				AssetHandle<Texture2D> texHandle = AssetManager::load<Texture2D>(textures["Metalness"].as<UUID>());
				material->setTexture(TextureSlot::Metalness, texHandle);
			}

			if (textures["Roughness"] && textures["Roughness"].as<UUID>().isValid()) {
				AssetHandle<Texture2D> texHandle = AssetManager::load<Texture2D>(textures["Roughness"].as<UUID>());
				material->setTexture(TextureSlot::Roughness, texHandle);
			}

			if (textures["Occlusion"] && textures["Occlusion"].as<UUID>().isValid()) {
				AssetHandle<Texture2D> texHandle = AssetManager::load<Texture2D>(textures["Occlusion"].as<UUID>());
				material->setTexture(TextureSlot::Occlusion, texHandle);
			}

			if (textures["Emissive"] && textures["Emissive"].as<UUID>().isValid()) {
				AssetHandle<Texture2D> texHandle = AssetManager::load<Texture2D>(textures["Emissive"].as<UUID>());
				material->setTexture(TextureSlot::Emissive, texHandle);
			}
		}

		AX_CORE_LOG_INFO("Reloaded Material: {}", data["Name"].as<std::string>());
	}

	// ----- Texture2D Assets -----
	template<>
	AssetHandle<Texture2D> AssetManager::load<Texture2D>(UUID handle) {
		if (has<Texture2D>(handle)) return handle;

		auto registry = ProjectManager::getProject()->getAssetRegistry();
		if (!registry->contains(handle)) {
			AX_CORE_LOG_ERROR("Texture2D UUID not found in AssetRegistry!");
			return {};
		}

		std::string absolutePath = getAbsolute(registry->get(handle).filePath.string());

		// -- Load Binary if in Runtime Mode --
		if (ProjectManager::isRuntime()) {
			storage<Texture2D>().assets[handle] = nullptr;
			storage<Texture2D>().loadQueue.push_back({ handle,
				[absolutePath]() {
					std::ifstream in(absolutePath, std::ios::in | std::ios::binary);

					BinaryAssetHeader header;
					in.read(reinterpret_cast<char*>(&header), sizeof(BinaryAssetHeader));

					uint64_t dataSize;
					in.read(reinterpret_cast<char*>(&dataSize), sizeof(uint64_t));

					std::vector<uint8_t> imageData(dataSize);
					in.read(reinterpret_cast<char*>(imageData.data()), dataSize);

					return Texture2D::create(imageData.data(), imageData.size());
				}
				});
			storage<Texture2D>().handleToPath[handle] = absolutePath;
			return handle;
		}

		// -- Load YAML if not in Runtime Mode --
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
		if (version == ASSET_VERSION_TEXTURE2D) {
			std::string sourcePath = getAbsolute(data["Source"].as<std::string>());
			UUID uuid = data["UUID"].as<UUID>();

			storage<Texture2D>().assets[handle] = nullptr;
			storage<Texture2D>().loadQueue.push_back({ handle,
				[sourcePath, handle]() {
					return Texture2D::create(sourcePath);
				}
				});
			storage<Texture2D>().handleToPath[handle] = absolutePath;

			return handle;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported Texture2D Version: {} in file {}", version, absolutePath);
			return {};
		}

	}

	// ----- AudioClip Assets -----
	template<>
	AssetHandle<AudioClip> AssetManager::load<AudioClip>(UUID handle) {
		if (has<AudioClip>(handle)) return handle;

		auto registry = ProjectManager::getProject()->getAssetRegistry();
		if (!registry->contains(handle)) {
			AX_CORE_LOG_ERROR("AudioClip UUID not found in AssetRegistry!");
			return {};
		}

		std::string absolutePath = getAbsolute(registry->get(handle).filePath.string());

		// -- Load Binary if in Runtime Mode --
		if (ProjectManager::isRuntime()) {
			storage<AudioClip>().assets[handle] = nullptr;
			storage<AudioClip>().loadQueue.push_back({ handle,
				[absolutePath]() {
					std::ifstream in(absolutePath, std::ios::in | std::ios::binary);

					BinaryAssetHeader header;
					in.read(reinterpret_cast<char*>(&header), sizeof(BinaryAssetHeader));

					uint32_t modeInt;
					in.read(reinterpret_cast<char*>(&modeInt), sizeof(uint32_t));
					AudioClip::Mode mode = static_cast<AudioClip::Mode>(modeInt);

					uint64_t dataSize;
					in.read(reinterpret_cast<char*>(&dataSize), sizeof(uint64_t));

					std::vector<uint8_t> audioData(dataSize);
					in.read(reinterpret_cast<char*>(audioData.data()), dataSize);

					return std::make_shared<AudioClip>(std::move(audioData), mode);
				}
			});
			storage<AudioClip>().handleToPath[handle] = absolutePath;
			return handle;
		}

		// -- Load YAML --
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
		if (version == ASSET_VERSION_AUDIO) {
			std::string sourcePath = getAbsolute(data["Source"].as<std::string>());
			UUID uuid = data["UUID"].as<UUID>();

			AudioClip::Mode mode = EnumUtils::AudioClipModeFromString(data["Mode"].as<std::string>());
			Ref<AudioClip> clip = std::make_shared<AudioClip>(sourcePath, mode);

			storage<AudioClip>().assets[handle] = clip;
			storage<AudioClip>().handleToPath[handle] = absolutePath;

			return handle;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported AudioClip Version: {} in file {}", version, absolutePath);
			return {};
		}

	}

	// ----- PhysicsMaterial -----
	template<>
	AssetHandle<PhysicsMaterial> AssetManager::load<PhysicsMaterial>(UUID handle) {
		if (has<PhysicsMaterial>(handle)) return handle;

		auto registry = ProjectManager::getProject()->getAssetRegistry();
		if (!registry->contains(handle)) {
			AX_CORE_LOG_ERROR("PhysicsMaterial UUID not found in AssetRegistry!");
			return {};
		}

		std::string absolutePath = getAbsolute(registry->get(handle).filePath.string());

		// -- Load Binary if in Runtime Mode --
		if (ProjectManager::isRuntime()) {
			storage<PhysicsMaterial>().assets[handle] = nullptr;
			storage<PhysicsMaterial>().loadQueue.push_back({ handle,
				[absolutePath]() {
					std::ifstream in(absolutePath, std::ios::in | std::ios::binary);

					BinaryAssetHeader header;
					in.read(reinterpret_cast<char*>(&header), sizeof(BinaryAssetHeader));

					Ref<PhysicsMaterial> material = std::make_shared<PhysicsMaterial>();

					in.read(reinterpret_cast<char*>(&material->staticFriction), sizeof(float));
					in.read(reinterpret_cast<char*>(&material->dynamicFriction), sizeof(float));
					in.read(reinterpret_cast<char*>(&material->restitution), sizeof(float));

					return material;
				}
			});
			storage<PhysicsMaterial>().handleToPath[handle] = absolutePath;
			return handle;
		}

		// -- Load YAML if not in Runtime Mode --
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
		if (version == ASSET_VERSION_PHYSICS_MATERIAL) {
			UUID uuid = data["UUID"].as<UUID>();

			Ref<PhysicsMaterial> material = std::make_shared<PhysicsMaterial>();

			material->staticFriction = data["StaticFriction"].as<float>();
			material->dynamicFriction = data["DynamicFriction"].as<float>();
			material->restitution = data["Restitution"].as<float>();

			storage<PhysicsMaterial>().assets[handle] = material;
			storage<PhysicsMaterial>().handleToPath[handle] = absolutePath;

			return handle;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported PhysivsMaterial Version: {} in file {}", version, absolutePath);
			return {};
		}

	}

	// -- Prefab --
	template<>
	AssetHandle<Prefab> AssetManager::load<Prefab>(UUID handle) {
		if (has<Prefab>(handle)) return handle;

		auto registry = ProjectManager::getProject()->getAssetRegistry();
		if (!registry->contains(handle)) {
			AX_CORE_LOG_ERROR("Prefab UUID not found in AssetRegistry!");
			return {};
		}

		std::string absolutePath = getAbsolute(registry->get(handle).filePath.string());
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
		if (version == ASSET_VERSION_PREFAB) {
			UUID uuid = data["UUID"].as<UUID>();

			YAML::Node entityNode = data["Entity"];
			Ref<Prefab> prefab = std::make_shared<Prefab>(entityNode);

			storage<Prefab>().assets[handle] = prefab;
			storage<Prefab>().handleToPath[handle] = absolutePath;

			return handle;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported Prefab Version: {} in file {}", version, absolutePath);
			return {};
		}

	}

}
