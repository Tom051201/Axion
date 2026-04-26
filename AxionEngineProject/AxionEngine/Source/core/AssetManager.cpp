#include "axpch.h"
#include "AssetManager.h"

#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/core/BinaryHeaders.h"
#include "AxionEngine/Source/core/EngineAssets.h"
#include "AxionEngine/Source/core/BinaryAssetLoader.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/scene/Skybox.h"
#include "AxionEngine/Source/scene/Prefab.h"
#include "AxionEngine/Source/scene/Animation.h"
#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Shader.h"
#include "AxionEngine/Source/render/Material.h"
#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/render/Texture.h"
#include "AxionEngine/Source/render/SkeletalMesh.h"
#include "AxionEngine/Source/audio/AudioClip.h"
#include "AxionEngine/Source/physics/PhysicsMaterial.h"

namespace Axion {

	void AssetManager::initialize(AssetLoader* loader) {
		if (loader != nullptr) {
			s_loader = loader;
			AX_CORE_LOG_TRACE("AssetManager initialized with custom loader");
		}
		else {
			s_loader = new BinaryAssetLoader();
			AX_CORE_LOG_TRACE("AssetManager initialized with default binary loader");
		}
	}

	void AssetManager::shutdown() {
		delete s_loader;
		s_loader = nullptr;

		release<Mesh>();
		release<TextureCube>();
		release<Skybox>();
		release<Shader>();
		release<Material>();
		release<AudioClip>();
		release<Texture2D>();
		release<Pipeline>();
		release<PhysicsMaterial>();
		release<SkeletalMesh>();
		release<AnimationClip>();
		release<Prefab>();

		AX_CORE_LOG_INFO("AssetManager shutdown");
	}

	void AssetManager::onEvent(Event& e) {
		// -- RenderingFinished --
		if (e.getEventType() == EventType::RenderingFinished) {

			processLoadQueue<Skybox>();
			processLoadQueue<TextureCube>();
			processLoadQueue<Shader>();
			processLoadQueue<Pipeline>();
			processLoadQueue<Mesh>();
			processLoadQueue<Texture2D>();
			processLoadQueue<Material>();
			processLoadQueue<AudioClip>();
			processLoadQueue<PhysicsMaterial>();
			processLoadQueue<SkeletalMesh>();
			processLoadQueue<AnimationClip>();
			processLoadQueue<Prefab>();

		}
	}

	std::filesystem::path AssetManager::getRelativeToAssets(const std::filesystem::path& absolutePath) {
		if (!ProjectManager::hasProject()) {
			AX_CORE_LOG_WARN("Unable converting absolute path to relative assets path: no project loaded");
			return {};
		}

		std::filesystem::path assetsDir = ProjectManager::getProject()->getAssetsPath();

		try {
			return std::filesystem::relative(absolutePath, assetsDir);
		}
		catch (const std::exception& e) {
			AX_CORE_LOG_WARN("Failed to convert absolute path to relative: {}", e.what());
			(void)e;
			return {};
		}
	}

	std::filesystem::path AssetManager::getAbsolute(const std::filesystem::path& relativePath) {
		if (!ProjectManager::hasProject()) {
			AX_CORE_LOG_WARN("Unable converting relative path to absolute assets path: no project loaded");
			return {};
		}

		std::filesystem::path absPath = ProjectManager::getProject()->getAssetsPath() / relativePath;

		if (!std::filesystem::exists(absPath)) {
			AX_CORE_LOG_WARN("Unable converting relative path to absolute assets path: path does not exist");
			return {};
		}

		return absPath;
	}

	UUID AssetManager::getAssetUUID(const std::filesystem::path& absolutePath) {
		return s_loader->peekUUID(absolutePath);
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

		s_loader->loadMesh(handle, getAbsolute(registry->get(handle).filePath));
		return handle;
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

		s_loader->loadTextureCube(handle, getAbsolute(registry->get(handle).filePath));
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

		s_loader->loadSkybox(handle, getAbsolute(registry->get(handle).filePath));
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

		s_loader->loadShader(handle, getAbsolute(registry->get(handle).filePath));
		return handle;
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

		s_loader->loadPipeline(handle, getAbsolute(registry->get(handle).filePath));
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

		s_loader->loadMaterial(handle, getAbsolute(registry->get(handle).filePath));
		return handle;
	}

	template<>
	void AssetManager::reload<Material>(const AssetHandle<Material>& handle) {
		if (!has<Material>(handle)) {
			AX_CORE_LOG_WARN("Cannot reload material: Handle not found in registry");
			return;
		}

		s_loader->reloadMaterial(handle.uuid, getAssetFilePath(handle));
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

		s_loader->loadTexture2D(handle, getAbsolute(registry->get(handle).filePath));
		return handle;
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

		s_loader->loadAudioClip(handle, getAbsolute(registry->get(handle).filePath));
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

		s_loader->loadPhysicsMaterial(handle, getAbsolute(registry->get(handle).filePath));
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

		s_loader->loadPrefab(handle, getAbsolute(registry->get(handle).filePath));
		return handle;
	}

	// -- SkeletalMesh --
	template<>
	AssetHandle<SkeletalMesh> AssetManager::load<SkeletalMesh>(UUID handle) {
		if (has<SkeletalMesh>(handle)) return handle;

		auto registry = ProjectManager::getProject()->getAssetRegistry();
		if (!registry->contains(handle)) {
			AX_CORE_LOG_ERROR("Prefab UUID not found in AssetRegistry!");
			return {};
		}

		s_loader->loadSkeletalMesh(handle, getAbsolute(registry->get(handle).filePath));
		return handle;
	}

	// -- AnimationClip --
	template<>
	AssetHandle<AnimationClip> AssetManager::load<AnimationClip>(UUID handle) {
		if (has<AnimationClip>(handle)) return handle;

		auto registry = ProjectManager::getProject()->getAssetRegistry();
		if (!registry->contains(handle)) {
			AX_CORE_LOG_ERROR("Prefab UUID not found in AssetRegistry!");
			return {};
		}

		s_loader->loadAnimationClip(handle, getAbsolute(registry->get(handle).filePath));
		return handle;
	}

}
