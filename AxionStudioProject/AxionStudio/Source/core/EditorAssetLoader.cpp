#include "studiopch.h"
#include "EditorAssetLoader.h"

#include "AxionEngine/Source/core/YamlHelper.h"
#include "AxionEngine/Source/core/EnumUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/core/JobSystem.h"
#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/graphics/Mesh.h"
#include "AxionEngine/Source/graphics/Shader.h"
#include "AxionEngine/Source/graphics/Material.h"
#include "AxionEngine/Source/graphics/Renderer.h"
#include "AxionEngine/Source/graphics/Texture.h"
#include "AxionEngine/Source/graphics/SkeletalMesh.h"
#include "AxionEngine/Source/scene/Skybox.h"
#include "AxionEngine/Source/scene/Prefab.h"
#include "AxionEngine/Source/scene/Animation.h"
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
		AssetManager::storage<Mesh>().assets[handle] = nullptr;
		AssetManager::storage<Mesh>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- Background Thread --
			std::ifstream stream(absolutePath);
			if (!stream.is_open()) {
				AX_CORE_LOG_ERROR("Failed to open mesh asset file: {}", absolutePath.string());
				return;
			}

			YAML::Node data;
			try {
				data = YAML::Load(stream);
			}
			catch (const YAML::Exception& e) {
				AX_CORE_LOG_ERROR("Failed to parse mesh YAML {}: {}", absolutePath.string(), e.what());
				return;
			}

			uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
			if (version > ASSET_VERSION_MESH) {
				AX_CORE_LOG_ERROR("Unsupported Mesh Version: {} in file {}", version, absolutePath.string());
				return;
			}

			std::filesystem::path sourcePath = AssetManager::getAbsolute(data["Source"].as<std::string>());
			std::string format = data["Format"] ? data["Format"].as<std::string>() : "OBJ";

			auto meshData = std::make_shared<MeshData>();

			// -- Extract Mesh Off The Main Thread --
			if (format == "GLB" || format == "GLTF") {
				*meshData = AAP::GLTFImporter::extractMeshes(sourcePath);
			}
			else {
				*meshData = AAP::OBJImporter::extractMeshes(sourcePath);
			}

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<Mesh>(handle, [meshData]() -> Ref<Mesh> {
				return Mesh::create(*meshData);
			});

		});
	}

	void EditorAssetLoader::loadTexture2D(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<Texture2D>().assets[handle] = nullptr;
		AssetManager::storage<Texture2D>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {
			// -- Background Thread --
			std::ifstream stream(absolutePath);
			if (!stream.is_open()) {
				AX_CORE_LOG_ERROR("Failed to open texture asset file: {}", absolutePath.string());
				return;
			}

			// -- Parse YAML Off The Main Thread --
			YAML::Node data;
			try {
				data = YAML::Load(stream);
			}
			catch (const YAML::Exception& e) {
				AX_CORE_LOG_ERROR("Failed to parse texture YAML {}: {}", absolutePath.string(), e.what());
				return;
			}

			uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
			if (version != ASSET_VERSION_TEXTURE2D) {
				AX_CORE_LOG_ERROR("Unsupported Texture2D Version: {} in file {}", version, absolutePath.string());
				return;
			}

			std::filesystem::path sourcePath = AssetManager::getAbsolute(data["Source"].as<std::string>());
			if (!std::filesystem::exists(sourcePath)) {
				AX_CORE_LOG_ERROR("Texture source file missing: {}", sourcePath.string());
				return;
			}

			// -- Read Raw Image File From Disk Into RAM --
			std::ifstream imageStream(sourcePath, std::ios::in | std::ios::binary);
			imageStream.seekg(0, std::ios::end);
			size_t fileSize = imageStream.tellg();
			imageStream.seekg(0, std::ios::beg);

			auto imageData = std::make_shared<std::vector<uint8_t>>(fileSize);
			imageStream.read(reinterpret_cast<char*>(imageData->data()), fileSize);

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<Texture2D>(handle, [imageData]() -> Ref<Texture2D> {
				return Texture2D::create(imageData->data(), imageData->size());
			});
		});
	}

	void EditorAssetLoader::loadTextureCube(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<TextureCube>().assets[handle] = nullptr;
		AssetManager::storage<TextureCube>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- Background Thread --
			std::ifstream stream(absolutePath);
			if (!stream.is_open()) {
				AX_CORE_LOG_ERROR("Failed to open TextureCube asset file: {}", absolutePath.string());
				return;
			}

			YAML::Node data;
			try {
				data = YAML::Load(stream);
			}
			catch (const YAML::Exception& e) {
				AX_CORE_LOG_ERROR("Failed to parse TextureCube YAML {}: {}", absolutePath.string(), e.what());
				return;
			}

			uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
			if (version != ASSET_VERSION_TEXTURE_CUBE) {
				AX_CORE_LOG_ERROR("Unsupported TextureCube Version: {} in file {}", version, absolutePath.string());
				return;
			}

			std::filesystem::path sourcePath = AssetManager::getAbsolute(data["Source"].as<std::string>());
			if (!std::filesystem::exists(sourcePath)) {
				AX_CORE_LOG_ERROR("TextureCube source file missing: {}", sourcePath.string());
				return;
			}

			// -- Read Raw Image File From Disk Into RAM --
			std::ifstream imageStream(sourcePath, std::ios::in | std::ios::binary);
			imageStream.seekg(0, std::ios::end);
			size_t fileSize = imageStream.tellg();
			imageStream.seekg(0, std::ios::beg);

			auto imageData = std::make_shared<std::vector<uint8_t>>(fileSize);
			imageStream.read(reinterpret_cast<char*>(imageData->data()), fileSize);

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<TextureCube>(handle, [imageData]() -> Ref<TextureCube> {
				return TextureCube::create(imageData->data(), imageData->size());
			});

		});
	}

	void EditorAssetLoader::loadSkybox(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<Skybox>().assets[handle] = nullptr;
		AssetManager::storage<Skybox>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- YAML On Background Thread --
			std::ifstream stream(absolutePath);
			if (!stream.is_open()) {
				AX_CORE_LOG_ERROR("Failed to open Skybox asset file: {}", absolutePath.string());
				return;
			}

			YAML::Node data;
			try {
				data = YAML::Load(stream);
			}
			catch (const YAML::Exception& e) {
				AX_CORE_LOG_ERROR("Failed to parse Skybox YAML {}: {}", absolutePath.string(), e.what());
				return;
			}

			uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
			if (version != ASSET_VERSION_SKYBOX) {
				AX_CORE_LOG_ERROR("Unsupported Skybox Version: {} in file {}", version, absolutePath.string());
				return;
			}

			UUID texUUID = data["TextureCube"].as<UUID>();
			UUID pipeUUID = UUID(0, 0);
			if (data["Pipeline"]) {
				pipeUUID = data["Pipeline"].as<UUID>();
			}

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<Skybox>(handle, [texUUID, pipeUUID]() -> Ref<Skybox> {

				AssetHandle<TextureCube> texHandle = AssetManager::load<TextureCube>(texUUID);

				if (pipeUUID.isValid()) {
					AssetHandle<Pipeline> pipeHandle = AssetManager::load<Pipeline>(pipeUUID);
					return std::make_shared<Skybox>(texHandle, pipeHandle);
				}
				else {
					return std::make_shared<Skybox>(texHandle);
				}
			});

		});
	}

	void EditorAssetLoader::loadShader(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<Shader>().assets[handle] = nullptr;
		AssetManager::storage<Shader>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- YAML And Compilation On Background Thread --
			std::ifstream stream(absolutePath);
			if (!stream.is_open()) {
				AX_CORE_LOG_ERROR("Failed to open Shader asset file: {}", absolutePath.string());
				return;
			}

			YAML::Node data;
			try {
				data = YAML::Load(stream);
			}
			catch (const YAML::Exception& e) {
				AX_CORE_LOG_ERROR("Failed to parse Shader YAML {}: {}", absolutePath.string(), e.what());
				return;
			}

			uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
			if (version != ASSET_VERSION_SHADER) {
				AX_CORE_LOG_ERROR("Unsupported Shader Version: {} in file {}", version, absolutePath.string());
				return;
			}

			std::filesystem::path sourcePath = AssetManager::getAbsolute(data["Source"].as<std::string>());

			YAML::Node specData = data["Specification"];
			ShaderSpecification spec = {};
			spec.name = data["Name"].as<std::string>();
			if (specData["BatchTextures"]) {
				spec.batchTextures = specData["BatchTextures"].as<uint32_t>();
			}

			// -- Compiling On Background Thread --
			auto bytecode = std::make_shared<ShaderBytecode>(Shader::compileToBytecode(sourcePath));

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<Shader>(handle, [spec, sourcePath, bytecode]() -> Ref<Shader> {
				if (!bytecode->isValid()) {
					AX_CORE_LOG_ERROR("Failed to compile shader bytecode for: {}", sourcePath.string());
					return nullptr;
				}

				Ref<Shader> shader = Shader::create(spec, sourcePath);
				shader->loadFromBytecode(bytecode->vertex.data(), bytecode->vertex.size(), bytecode->pixel.data(), bytecode->pixel.size());
				return shader;
			});

		});
	}

	void EditorAssetLoader::loadPipeline(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<Pipeline>().assets[handle] = nullptr;
		AssetManager::storage<Pipeline>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- YAML And Parsing On Background Thread --
			std::ifstream stream(absolutePath);
			if (!stream.is_open()) {
				AX_CORE_LOG_ERROR("Failed to open Pipeline asset file: {}", absolutePath.string());
				return;
			}

			YAML::Node data;
			try {
				data = YAML::Load(stream);
			}
			catch (const YAML::Exception& e) {
				AX_CORE_LOG_ERROR("Failed to parse Pipeline YAML {}: {}", absolutePath.string(), e.what());
				return;
			}

			uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
			if (version != ASSET_VERSION_PIPELINE) {
				AX_CORE_LOG_ERROR("Unsupported Pipeline Version: {} in file {}", version, absolutePath.string());
				return;
			}

			YAML::Node specData = data["Specification"];
			UUID shaderUUID = specData["Shader"].as<UUID>();

			auto spec = std::make_shared<PipelineSpecification>();
			spec->numRenderTargets = specData["NumRenderTargets"].as<uint32_t>();
			spec->colorFormat = EnumUtils::colorFormatFromString(specData["ColorFormat"].as<std::string>());
			spec->depthStencilFormat = EnumUtils::depthStencilFormatFromString(specData["DepthStencilFormat"].as<std::string>());
			spec->depthTest = specData["DepthTest"].as<bool>();
			spec->depthWrite = specData["DepthWrite"].as<bool>();
			spec->depthFunction = EnumUtils::depthCompareFromString(specData["DepthFunction"].as<std::string>());
			spec->stencilEnabled = specData["StencilEnabled"].as<bool>();
			spec->sampleCount = specData["SampleCount"].as<uint32_t>();
			spec->cullMode = EnumUtils::cullModeFromString(specData["CullMode"].as<std::string>());
			spec->topology = EnumUtils::primitiveTopologyFromString(specData["Topology"].as<std::string>());

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
				spec->vertexLayout = layout;
			}

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<Pipeline>(handle, [spec, shaderUUID]() -> Ref<Pipeline> {
				AssetHandle<Shader> shaderHandle = AssetManager::load<Shader>(shaderUUID);
				spec->shader = AssetManager::get<Shader>(shaderHandle);

				AX_CORE_ASSERT(spec->shader, "Shader must be valid before creating pipeline!");

				return Pipeline::create(*spec);
			});

		});
	}

	void EditorAssetLoader::loadMaterial(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<Material>().assets[handle] = nullptr;
		AssetManager::storage<Material>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- YAML And Parsing On Background Thread --
			std::ifstream stream(absolutePath);
			if (!stream.is_open()) {
				AX_CORE_LOG_ERROR("Failed to open Material asset file: {}", absolutePath.string());
				return;
			}

			YAML::Node data;
			try {
				data = YAML::Load(stream);
			}
			catch (const YAML::Exception& e) {
				AX_CORE_LOG_ERROR("Failed to parse Material YAML {}: {}", absolutePath.string(), e.what());
				return;
			}

			uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
			if (version != ASSET_VERSION_MATERIAL) {
				AX_CORE_LOG_ERROR("Unsupported Material Version: {} in file {}", version, absolutePath.string());
				return;
			}

			std::string name = data["Name"].as<std::string>();

			auto prop = std::make_shared<MaterialProperties>();
			prop->albedoColor = data["AlbedoColor"].as<Vec4>();
			prop->metalness = data["Metalness"].as<float>();
			prop->roughness = data["Roughness"].as<float>();
			prop->emissionStrength = data["Emission"].as<float>();
			prop->tiling = data["Tiling"].as<float>();
			prop->useNormalMap = data["UseNormalMap"].as<float>();
			prop->useMetalnessMap = data["UseMetalnessMap"].as<float>();
			prop->useRoughnessMap = data["UseRoughnessMap"].as<float>();
			prop->useOcclusionMap = data["UseOcclusionMap"].as<float>();
			prop->useEmissiveMap = data["UseEmissiveMap"].as<float>();

			UUID pipelineUUID = UUID(0, 0);
			if (data["Pipeline"]) {
				pipelineUUID = data["Pipeline"].as<UUID>();
			}

			auto texturesToLoad = std::make_shared<std::vector<std::pair<TextureSlot, UUID>>>();

			if (data["Textures"]) {
				auto textures = data["Textures"];

				if (textures["Albedo"] && textures["Albedo"].as<UUID>().isValid())
					texturesToLoad->push_back({ TextureSlot::Albedo, textures["Albedo"].as<UUID>() });

				if (textures["Normal"] && textures["Normal"].as<UUID>().isValid())
					texturesToLoad->push_back({ TextureSlot::Normal, textures["Normal"].as<UUID>() });

				if (textures["Metalness"] && textures["Metalness"].as<UUID>().isValid())
					texturesToLoad->push_back({ TextureSlot::Metalness, textures["Metalness"].as<UUID>() });

				if (textures["Roughness"] && textures["Roughness"].as<UUID>().isValid())
					texturesToLoad->push_back({ TextureSlot::Roughness, textures["Roughness"].as<UUID>() });

				if (textures["Occlusion"] && textures["Occlusion"].as<UUID>().isValid())
					texturesToLoad->push_back({ TextureSlot::Occlusion, textures["Occlusion"].as<UUID>() });

				if (textures["Emissive"] && textures["Emissive"].as<UUID>().isValid())
					texturesToLoad->push_back({ TextureSlot::Emissive, textures["Emissive"].as<UUID>() });
			}

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<Material>(handle, [name, prop, pipelineUUID, texturesToLoad]() -> Ref<Material> {

				Ref<Material> material;

				if (pipelineUUID.isValid()) {
					AssetHandle<Pipeline> pipelineHandle = AssetManager::load<Pipeline>(pipelineUUID);
					material = Material::create(name, pipelineHandle, *prop);
				}
				else {
					material = Material::create(name, *prop);
				}

				for (const auto& texInfo : *texturesToLoad) {
					AssetHandle<Texture2D> handle = AssetManager::load<Texture2D>(texInfo.second);
					material->setTexture(texInfo.first, handle);
				}

				return material;
			});

		});
	}

	void EditorAssetLoader::loadAudioClip(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<AudioClip>().assets[handle] = nullptr;
		AssetManager::storage<AudioClip>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- YAML And Parsing On Background Thread --
			std::ifstream stream(absolutePath);
			if (!stream.is_open()) {
				AX_CORE_LOG_ERROR("Failed to open AudioClip asset file: {}", absolutePath.string());
				return;
			}

			YAML::Node data;
			try {
				data = YAML::Load(stream);
			}
			catch (const YAML::Exception& e) {
				AX_CORE_LOG_ERROR("Failed to parse AudioClip YAML {}: {}", absolutePath.string(), e.what());
				return;
			}

			uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
			if (version != ASSET_VERSION_AUDIO) {
				AX_CORE_LOG_ERROR("Unsupported AudioClip Version: {} in file {}", version, absolutePath.string());
				return;
			}

			std::filesystem::path sourcePath = AssetManager::getAbsolute(data["Source"].as<std::string>());
			AudioClip::Mode mode = EnumUtils::AudioClipModeFromString(data["Mode"].as<std::string>());

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<AudioClip>(handle, [sourcePath, mode]() -> Ref<AudioClip> {
				return std::make_shared<AudioClip>(sourcePath, mode);
			});

		});
	}

	void EditorAssetLoader::loadPhysicsMaterial(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<PhysicsMaterial>().assets[handle] = nullptr;
		AssetManager::storage<PhysicsMaterial>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- YAML And Parsing Background Thread --
			std::ifstream stream(absolutePath);
			if (!stream.is_open()) {
				AX_CORE_LOG_ERROR("Failed to open PhysicsMaterial asset file: {}", absolutePath.string());
				return;
			}

			YAML::Node data;
			try {
				data = YAML::Load(stream);
			}
			catch (const YAML::Exception& e) {
				AX_CORE_LOG_ERROR("Failed to parse PhysicsMaterial YAML {}: {}", absolutePath.string(), e.what());
				return;
			}

			uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
			if (version != ASSET_VERSION_PHYSICS_MATERIAL) {
				AX_CORE_LOG_ERROR("Unsupported PhysicsMaterial Version: {} in file {}", version, absolutePath.string());
				return;
			}

			float staticFriction = data["StaticFriction"].as<float>();
			float dynamicFriction = data["DynamicFriction"].as<float>();
			float restitution = data["Restitution"].as<float>();

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<PhysicsMaterial>(handle, [staticFriction, dynamicFriction, restitution]() -> Ref<PhysicsMaterial> {
				Ref<PhysicsMaterial> material = std::make_shared<PhysicsMaterial>();

				material->staticFriction = staticFriction;
				material->dynamicFriction = dynamicFriction;
				material->restitution = restitution;

				return material;
			});

		});
	}

	void EditorAssetLoader::loadPrefab(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<Prefab>().assets[handle] = nullptr;
		AssetManager::storage<Prefab>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- YAML And Parsing On Background Thread --
			std::ifstream stream(absolutePath);
			if (!stream.is_open()) {
				AX_CORE_LOG_ERROR("Failed to open Prefab asset file: {}", absolutePath.string());
				return;
			}

			YAML::Node data;
			try {
				data = YAML::Load(stream);
			}
			catch (const YAML::Exception& e) {
				AX_CORE_LOG_ERROR("Failed to parse Prefab YAML {}: {}", absolutePath.string(), e.what());
				return;
			}

			uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
			if (version != ASSET_VERSION_PREFAB) {
				AX_CORE_LOG_ERROR("Unsupported Prefab Version: {} in file {}", version, absolutePath.string());
				return;
			}

			auto entityNode = std::make_shared<YAML::Node>(data["Entity"]);

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<Prefab>(handle, [entityNode]() -> Ref<Prefab> {
				return std::make_shared<Prefab>(*entityNode);
			});

		});
	}

	void EditorAssetLoader::loadAnimationClip(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<AnimationClip>().assets[handle] = nullptr;
		AssetManager::storage<AnimationClip>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- YAML And GLTF Parsing Background Thread --
			std::ifstream stream(absolutePath);
			if (!stream.is_open()) {
				AX_CORE_LOG_ERROR("Failed to open AnimationClip asset file: {}", absolutePath.string());
				return;
			}

			YAML::Node data;
			try {
				data = YAML::Load(stream);
			}
			catch (const YAML::Exception& e) {
				AX_CORE_LOG_ERROR("Failed to parse AnimationClip YAML {}: {}", absolutePath.string(), e.what());
				return;
			}

			uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
			if (version != ASSET_VERSION_ANIMATION_CLIP) {
				AX_CORE_LOG_ERROR("Unsupported Animation Clip Version: {} in file {}", version, absolutePath.string());
				return;
			}

			std::filesystem::path sourcePath = AssetManager::getAbsolute(data["Source"].as<std::string>());

			Ref<AnimationClip> clip = AAP::GLTFImporter::extractAnimation(sourcePath);

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<AnimationClip>(handle, [clip]() -> Ref<AnimationClip> {
				return clip;
			});

		});
	}

	void EditorAssetLoader::loadSkeletalMesh(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<SkeletalMesh>().assets[handle] = nullptr;
		AssetManager::storage<SkeletalMesh>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- YAML And GLTF Parsing Background Thread --
			std::ifstream stream(absolutePath);
			if (!stream.is_open()) {
				AX_CORE_LOG_ERROR("Failed to open SkeletalMesh asset file: {}", absolutePath.string());
				return;
			}

			YAML::Node data;
			try {
				data = YAML::Load(stream);
			}
			catch (const YAML::Exception& e) {
				AX_CORE_LOG_ERROR("Failed to parse SkeletalMesh YAML {}: {}", absolutePath.string(), e.what());
				return;
			}

			uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;
			if (version > ASSET_VERSION_SKELETAL_MESH) {
				AX_CORE_LOG_ERROR("Unsupported Skeletal Mesh Version: {} in file {}", version, absolutePath.string());
				return;
			}

			std::filesystem::path sourcePath = AssetManager::getAbsolute(data["Source"].as<std::string>());

			auto meshData = std::make_shared<SkeletalMeshData>(AAP::GLTFImporter::extractSkeletalMesh(sourcePath));

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<SkeletalMesh>(handle, [meshData]() -> Ref<SkeletalMesh> {
				return SkeletalMesh::create(*meshData);
			});

		});
	}

	void EditorAssetLoader::reloadMaterial(UUID handle, const std::filesystem::path& absolutePath) {
		JobSystem::submit([handle, absolutePath]() {

			// -- YAML And Parsing On Background Thread --
			std::ifstream stream(absolutePath);
			if (!stream.is_open()) {
				AX_CORE_LOG_ERROR("Failed to open material file for reloading: {}", absolutePath.string());
				return;
			}

			YAML::Node data;
			try {
				data = YAML::Load(stream);
			}
			catch (const YAML::Exception& e) {
				AX_CORE_LOG_ERROR("Failed to parse Material YAML for reloading {}: {}", absolutePath.string(), e.what());
				return;
			}

			if (!data["Type"] || data["Type"].as<std::string>() != "Material") {
				AX_CORE_LOG_ERROR("Reloading material failed, file is not a material asset file");
				return;
			}

			std::string name = data["Name"] ? data["Name"].as<std::string>() : "Unknown";

			auto prop = std::make_shared<MaterialProperties>();
			prop->albedoColor = data["AlbedoColor"].as<Vec4>();
			prop->metalness = data["Metalness"].as<float>();
			prop->roughness = data["Roughness"].as<float>();
			prop->emissionStrength = data["Emission"].as<float>();
			prop->tiling = data["Tiling"].as<float>();
			prop->useNormalMap = data["UseNormalMap"].as<float>();
			prop->useMetalnessMap = data["UseMetalnessMap"].as<float>();
			prop->useRoughnessMap = data["UseRoughnessMap"].as<float>();
			prop->useOcclusionMap = data["UseOcclusionMap"].as<float>();
			prop->useEmissiveMap = data["UseEmissiveMap"].as<float>();

			auto texturesToLoad = std::make_shared<std::vector<std::pair<TextureSlot, UUID>>>();
			if (data["Textures"]) {
				auto textures = data["Textures"];
				if (textures["Albedo"] && textures["Albedo"].as<UUID>().isValid()) texturesToLoad->push_back({ TextureSlot::Albedo, textures["Albedo"].as<UUID>() });
				if (textures["Normal"] && textures["Normal"].as<UUID>().isValid()) texturesToLoad->push_back({ TextureSlot::Normal, textures["Normal"].as<UUID>() });
				if (textures["Metalness"] && textures["Metalness"].as<UUID>().isValid()) texturesToLoad->push_back({ TextureSlot::Metalness, textures["Metalness"].as<UUID>() });
				if (textures["Roughness"] && textures["Roughness"].as<UUID>().isValid()) texturesToLoad->push_back({ TextureSlot::Roughness, textures["Roughness"].as<UUID>() });
				if (textures["Occlusion"] && textures["Occlusion"].as<UUID>().isValid()) texturesToLoad->push_back({ TextureSlot::Occlusion, textures["Occlusion"].as<UUID>() });
				if (textures["Emissive"] && textures["Emissive"].as<UUID>().isValid()) texturesToLoad->push_back({ TextureSlot::Emissive, textures["Emissive"].as<UUID>() });
			}

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<Material>(handle, [handle, name, prop, texturesToLoad]() -> Ref<Material> {

				AssetHandle<Material> assetHandle;
				assetHandle.uuid = handle;
				Ref<Material> material = AssetManager::get<Material>(assetHandle);

				if (!material) {
					AX_CORE_LOG_WARN("Attempted to reload a material that isn't currently loaded in RAM!");
					return nullptr;
				}

				// -- Mutate Live Material Safely On Main Thread --
				material->setProperties(*prop);
				material->clearTextures();

				for (const auto& texInfo : *texturesToLoad) {
					AssetHandle<Texture2D> texHandle = AssetManager::load<Texture2D>(texInfo.second);
					material->setTexture(texInfo.first, texHandle);
				}

				AX_CORE_LOG_INFO("Reloaded Material: {}", name);

				return material;
			});

		});
	}

}
