#include "EditorAssetLoader.h"

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionEngine/Source/core/YamlHelper.h"
#include "AxionEngine/Source/core/EnumUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/scene/Skybox.h"
#include "AxionEngine/Source/scene/Prefab.h"
#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Shader.h"
#include "AxionEngine/Source/render/Material.h"
#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/render/Texture.h"
#include "AxionEngine/Source/audio/AudioClip.h"
#include "AxionEngine/Source/physics/PhysicsMaterial.h"

#include "AxionAssetPipeline/Source/importer/OBJImporter.h"
#include "AxionAssetPipeline/Source/importer/GLTFImporter.h"

namespace Axion {

	UUID EditorAssetLoader::peekUUID(const std::filesystem::path& absolutePath) {
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
			AX_CORE_LOG_WARN("Failed to read UUID from asset file '{0}': {1}", absolutePath.string(), e.what());
		}

		return {};
	}

	void EditorAssetLoader::loadMesh(UUID handle, const std::filesystem::path& absolutePath) {
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
		if (version <= ASSET_VERSION_MESH) {
			std::filesystem::path sourcePath = AssetManager::getAbsolute(data["Source"].as<std::string>());

			std::string format = "OBJ";
			if (data["Format"]) {
				format = data["Format"].as<std::string>();
			}

			AssetManager::storage<Mesh>().assets[handle] = nullptr;

			AssetManager::storage<Mesh>().loadQueue.push_back({ handle,
				[sourcePath, format]() {
					MeshData meshData;
					if (format == "GLB" || format == "GLTF") {
						meshData = AAP::GLTFImporter::extractMeshes(sourcePath);
					}
					else {
						meshData = AAP::OBJImporter::extractMeshes(sourcePath);
					}

					return Mesh::create(meshData);
				}
			});

			AssetManager::storage<Mesh>().handleToPath[handle] = absolutePath;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported Mesh Version: {} in file {}", version, absolutePath.string());
		}
	}

	void EditorAssetLoader::loadTexture2D(UUID handle, const std::filesystem::path& absolutePath) {
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
		if (version == ASSET_VERSION_TEXTURE2D) {
			std::filesystem::path sourcePath = AssetManager::getAbsolute(data["Source"].as<std::string>());
			UUID uuid = data["UUID"].as<UUID>();

			AssetManager::storage<Texture2D>().assets[handle] = nullptr;
			AssetManager::storage<Texture2D>().loadQueue.push_back({ handle,
				[sourcePath, handle]() {
					return Texture2D::create(sourcePath);
				}
			});
			AssetManager::storage<Texture2D>().handleToPath[handle] = absolutePath;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported Texture2D Version: {} in file {}", version, absolutePath.string());
		}
	}

	void EditorAssetLoader::loadTextureCube(UUID handle, const std::filesystem::path& absolutePath) {
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
		if (version == ASSET_VERSION_TEXTURE_CUBE) {
			std::filesystem::path sourcePath = AssetManager::getAbsolute(data["Source"].as<std::string>());
			UUID uuid = data["UUID"].as<UUID>();

			AssetManager::storage<TextureCube>().assets[handle] = nullptr;

			AssetManager::storage<TextureCube>().loadQueue.push_back({ handle,
				[sourcePath, handle]() {
					return TextureCube::create(sourcePath);
				}
			});

			AssetManager::storage<TextureCube>().handleToPath[handle] = absolutePath;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported TextureCube Version: {} in file {}", version, absolutePath.string());
		}
	}

	void EditorAssetLoader::loadSkybox(UUID handle, const std::filesystem::path& absolutePath) {
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
		if (version == ASSET_VERSION_SKYBOX) {

			UUID texUUID = data["TextureCube"].as<UUID>();
			UUID pipeUUID = UUID(0, 0);
			if (data["Pipeline"]) {
				pipeUUID = data["Pipeline"].as<UUID>();
			}

			AssetManager::storage<Skybox>().assets[handle] = nullptr;

			AssetManager::storage<Skybox>().loadQueue.push_back({ handle,
				[texUUID, pipeUUID]() {
					AssetHandle<TextureCube> texHandle = AssetManager::load<TextureCube>(texUUID);
					if (pipeUUID.isValid()) {
						AssetHandle<Pipeline> pipeHandle = AssetManager::load<Pipeline>(pipeUUID);
						return std::make_shared<Skybox>(texHandle, pipeHandle);
					}
					else {
						return std::make_shared<Skybox>(texHandle);
					}
				}
			});

			AssetManager::storage<Skybox>().handleToPath[handle] = absolutePath;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported Skybox Version: {} in file {}", version, absolutePath.string());
		}
	}

	void EditorAssetLoader::loadShader(UUID handle, const std::filesystem::path& absolutePath) {
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
		if (version == ASSET_VERSION_SHADER) {
			std::filesystem::path sourcePath = AssetManager::getAbsolute(data["Source"].as<std::string>());
			UUID uuid = data["UUID"].as<UUID>();

			// -- Check format --
			std::string format = data["Format"].as<std::string>();
			RendererAPI api = Renderer::getAPI();

			bool formatMatches =
				(format == "HLSL" && api == RendererAPI::DirectX12) ||
				(format == "GLSL" && api == RendererAPI::OpenGL3);

			// -- Check extension --
			std::string ext = sourcePath.extension().string();

			bool extensionMatches =
				(api == RendererAPI::DirectX12 && ext == "HLSL") ||
				(api == RendererAPI::OpenGL3 && ext == "GLSL");

			if (!formatMatches) {
				if (extensionMatches) {
					AX_CORE_LOG_WARN("Shader format '{}' in '{}' does not match current RendererAPI, but file extension '{}' is valid. Attempting to load anyway.", format, absolutePath.string(), ext);
				}
				else {
					AX_CORE_LOG_ERROR("Shader format '{}' in '{}' is not supported by current RendererAPI and file extension '{}' does not match expected format", format, absolutePath.string(), ext);
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
			AssetManager::storage<Shader>().assets[handle] = shader;
			AssetManager::storage<Shader>().loadQueue.push_back({ handle,
				[sourcePath, handle]() {
					Ref<Shader> shader = AssetManager::get<Shader>(handle);
					shader->compileFromFile(sourcePath);
					return shader;
				}
			});
			AssetManager::storage<Shader>().handleToPath[handle] = absolutePath;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported Shader Version: {} in file {}", version, absolutePath.string());
		}
	}

	void EditorAssetLoader::loadPipeline(UUID handle, const std::filesystem::path& absolutePath) {
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
					if (elemNode["Normalized"]) { elem.normalized = elemNode["Normalized"].as<bool>(); }
					if (elemNode["Instanced"]) { elem.instanced = elemNode["Instanced"].as<bool>(); }

					elements.push_back(elem);
				}

				BufferLayout layout(elements);
				layout.calculateOffsetAndStride();
				spec.vertexLayout = layout;
			}

			AssetManager::storage<Pipeline>().assets[handle] = nullptr;
			AssetManager::storage<Pipeline>().loadQueue.push_back({ handle,
				[shaderHandle, spec]() mutable {
					spec.shader = AssetManager::get<Shader>(shaderHandle);
					AX_CORE_ASSERT(spec.shader, "Shader must be valid before creating pipeline!");
					return Pipeline::create(spec);
				}
			});
			AssetManager::storage<Pipeline>().handleToPath[handle] = absolutePath;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported Pipeline Version: {} in file {}", version, absolutePath.string());
		}
	}

	void EditorAssetLoader::loadMaterial(UUID handle, const std::filesystem::path& absolutePath) {
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
			prop.useEmissiveMap = data["UseEmissiveMap"].as<float>();

			Ref<Material> material;
			UUID pipelineUUID = UUID(0, 0);
			if (data["Pipeline"]) {
				pipelineUUID = data["Pipeline"].as<UUID>();
				AssetHandle<Pipeline> pipelineHandle = AssetManager::load<Pipeline>(pipelineUUID);
				material = Material::create(name, pipelineHandle, prop);
			}
			else {
				material = Material::create(name, prop);
			}

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

			AssetManager::storage<Material>().assets[handle] = material;
			AssetManager::storage<Material>().handleToPath[handle] = absolutePath;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported Material Version: {} in file {}", version, absolutePath.string());
		}
	}

	void EditorAssetLoader::loadAudioClip(UUID handle, const std::filesystem::path& absolutePath) {
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
		if (version == ASSET_VERSION_AUDIO) {
			std::filesystem::path sourcePath = AssetManager::getAbsolute(data["Source"].as<std::string>());
			UUID uuid = data["UUID"].as<UUID>();

			AudioClip::Mode mode = EnumUtils::AudioClipModeFromString(data["Mode"].as<std::string>());
			Ref<AudioClip> clip = std::make_shared<AudioClip>(sourcePath, mode);

			AssetManager::storage<AudioClip>().assets[handle] = clip;
			AssetManager::storage<AudioClip>().handleToPath[handle] = absolutePath;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported AudioClip Version: {} in file {}", version, absolutePath.string());
		}
	}

	void EditorAssetLoader::loadPhysicsMaterial(UUID handle, const std::filesystem::path& absolutePath) {
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
		if (version == ASSET_VERSION_PHYSICS_MATERIAL) {
			UUID uuid = data["UUID"].as<UUID>();

			Ref<PhysicsMaterial> material = std::make_shared<PhysicsMaterial>();

			material->staticFriction = data["StaticFriction"].as<float>();
			material->dynamicFriction = data["DynamicFriction"].as<float>();
			material->restitution = data["Restitution"].as<float>();

			AssetManager::storage<PhysicsMaterial>().assets[handle] = material;
			AssetManager::storage<PhysicsMaterial>().handleToPath[handle] = absolutePath;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported PhysivsMaterial Version: {} in file {}", version, absolutePath.string());
		}
	}

	void EditorAssetLoader::loadPrefab(UUID handle, const std::filesystem::path& absolutePath) {
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
		if (version == ASSET_VERSION_PREFAB) {
			UUID uuid = data["UUID"].as<UUID>();

			YAML::Node entityNode = data["Entity"];
			Ref<Prefab> prefab = std::make_shared<Prefab>(entityNode);

			AssetManager::storage<Prefab>().assets[handle] = prefab;
			AssetManager::storage<Prefab>().handleToPath[handle] = absolutePath;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported Prefab Version: {} in file {}", version, absolutePath.string());
		}
	}

	void EditorAssetLoader::reloadMaterial(UUID handle, const std::filesystem::path& absolutePath) {
		std::ifstream stream(absolutePath);
		if (!stream.is_open()) {
			AX_CORE_LOG_ERROR("Failed to open material file for reloading: {}", absolutePath.string());
			return;
		}

		YAML::Node data = YAML::Load(stream);
		if (data["Type"].as<std::string>() != "Material") {
			AX_CORE_LOG_ERROR("Reloading material failed, file is not a material asset file");
			return;
		}

		AssetHandle<Material> assetHandle;
		assetHandle.uuid = handle;
		Ref<Material> material = AssetManager::get<Material>(assetHandle);

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
		prop.useEmissiveMap = data["UseEmissiveMap"].as<float>();

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

}
