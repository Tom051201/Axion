#include "axpch.h"
#include "AssetManager.h"

#include "AxionEngine/Source/core/YamlHelper.h"
#include "AxionEngine/Source/core/EnumUtils.h"
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
			(void)e; // Prevents compiler warning
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
			(void)e; // Prevents compiler warning
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
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

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
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		std::string texturePath = getAbsolute(data["Texture"].as<std::string>());
		UUID pipelineUUID = data["Pipeline"].as<UUID>();
		UUID uuid = data["UUID"].as<UUID>();

		storage<Skybox>().assets[handle] = nullptr;
		storage<Skybox>().loadQueue.push_back({ handle,
			[texturePath, pipelineUUID]() {
				return std::make_shared<Skybox>(texturePath, pipelineUUID);
			}
		});
		storage<Skybox>().handleToPath[handle] = absolutePath;

		return handle;
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
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

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
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

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

	template<>
	AssetHandle<Pipeline> AssetManager::load<Pipeline>(UUID handle) {
		if (has<Pipeline>(handle)) return handle;

		auto registry = ProjectManager::getProject()->getAssetRegistry();
		if (!registry->contains(handle)) {
			AX_CORE_LOG_ERROR("Pipeline UUID not found in AssetRegistry!");
			return {};
		}

		std::string absolutePath = getAbsolute(registry->get(handle).filePath.string());
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

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
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

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
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		std::string sourcePath = getAbsolute(data["Source"].as<std::string>());
		UUID uuid = data["UUID"].as<UUID>();

		AudioClip::Mode mode = EnumUtils::AudioClipModeFromString(data["Mode"].as<std::string>());
		Ref<AudioClip> clip = std::make_shared<AudioClip>(sourcePath, mode);

		storage<AudioClip>().assets[handle] = clip;
		storage<AudioClip>().handleToPath[handle] = absolutePath;

		return handle;
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
		std::ifstream stream(absolutePath);
		YAML::Node data = YAML::Load(stream);

		UUID uuid = data["UUID"].as<UUID>();

		Ref<PhysicsMaterial> material = std::make_shared<PhysicsMaterial>();

		material->staticFriction = data["StaticFriction"].as<float>();
		material->dynamicFriction = data["DynamicFriction"].as<float>();
		material->restitution = data["Restitution"].as<float>();

		storage<PhysicsMaterial>().assets[handle] = material;
		storage<PhysicsMaterial>().handleToPath[handle] = absolutePath;

		return handle;
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


		UUID uuid = data["UUID"].as<UUID>();

		YAML::Node entityNode = data["Entity"];
		Ref<Prefab> prefab = std::make_shared<Prefab>(entityNode);

		storage<Prefab>().assets[handle] = prefab;
		storage<Prefab>().handleToPath[handle] = absolutePath;

		return handle;
	}

}
