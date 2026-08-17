#include "axpch.h"
#include "AssetManager.h"

#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/core/BinaryHeaders.h"
#include "AxionEngine/Source/core/EngineAssets.h"
#include "AxionEngine/Source/core/BinaryAssetLoader.h"
#include "AxionEngine/Source/graphics/Mesh.h"
#include "AxionEngine/Source/graphics/Shader.h"
#include "AxionEngine/Source/graphics/Material.h"
#include "AxionEngine/Source/graphics/Renderer.h"
#include "AxionEngine/Source/graphics/Texture.h"
#include "AxionEngine/Source/graphics/SkeletalMesh.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/scene/Skybox.h"
#include "AxionEngine/Source/scene/Prefab.h"
#include "AxionEngine/Source/scene/Animation.h"
#include "AxionEngine/Source/audio/AudioClip.h"
#include "AxionEngine/Source/physics/PhysicsMaterial.h"

namespace Axion {

	static uint32_t s_maxAssetsPerFrame = 2;

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

			uint32_t maxItemsThisFrame = s_maxAssetsPerFrame;
			processLoadQueue<Skybox>(maxItemsThisFrame);
			processLoadQueue<TextureCube>(maxItemsThisFrame);
			processLoadQueue<Shader>(maxItemsThisFrame);
			processLoadQueue<Pipeline>(maxItemsThisFrame);
			processLoadQueue<Mesh>(maxItemsThisFrame);
			processLoadQueue<Texture2D>(maxItemsThisFrame);
			processLoadQueue<Material>(maxItemsThisFrame);
			processLoadQueue<AudioClip>(maxItemsThisFrame);
			processLoadQueue<PhysicsMaterial>(maxItemsThisFrame);
			processLoadQueue<SkeletalMesh>(maxItemsThisFrame);
			processLoadQueue<AnimationClip>(maxItemsThisFrame);
			processLoadQueue<Prefab>(maxItemsThisFrame);

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

	void AssetManager::removeAsset(UUID handle) {
		if (!ProjectManager::hasProject() || !handle.isValid()) return;

		auto registry = ProjectManager::getProject()->getAssetRegistry();

		if (registry->contains(handle)) {
			AssetType type = registry->get(handle).type;

			// -- Remove from Registry File --
			registry->remove(handle);
			AX_CORE_LOG_INFO("Removed asset from registry: {}", handle.toString());

			// -- Clear From Live RAM --
			switch (type) {
				case AssetType::Mesh:				removeAssetFromStorage<Mesh>(handle); break;
				case AssetType::Texture2D:			removeAssetFromStorage<Texture2D>(handle); break;
				case AssetType::TextureCube:		removeAssetFromStorage<TextureCube>(handle); break;
				case AssetType::Material:			removeAssetFromStorage<Material>(handle); break;
				case AssetType::Shader:				removeAssetFromStorage<Shader>(handle); break;
				case AssetType::Pipeline:			removeAssetFromStorage<Pipeline>(handle); break;
				case AssetType::Skybox:				removeAssetFromStorage<Skybox>(handle); break;
				case AssetType::AudioClip:			removeAssetFromStorage<AudioClip>(handle); break;
				case AssetType::PhysicsMaterial:	removeAssetFromStorage<PhysicsMaterial>(handle); break;
				case AssetType::Prefab:				removeAssetFromStorage<Prefab>(handle); break;
				case AssetType::SkeletalMesh:		removeAssetFromStorage<SkeletalMesh>(handle); break;
				case AssetType::AnimationClip:		removeAssetFromStorage<AnimationClip>(handle); break;
				default: break;
			}
		}
	}

	bool AssetManager::isLoadingAssets() {
		size_t total = 0;
		total += getPendingCount<Skybox>();
		total += getPendingCount<TextureCube>();
		total += getPendingCount<Shader>();
		total += getPendingCount<Pipeline>();
		total += getPendingCount<Mesh>();
		total += getPendingCount<Texture2D>();
		total += getPendingCount<Material>();
		total += getPendingCount<AudioClip>();
		total += getPendingCount<PhysicsMaterial>();
		total += getPendingCount<SkeletalMesh>();
		total += getPendingCount<AnimationClip>();
		total += getPendingCount<Prefab>();
		return total > 0;
	}

	void AssetManager::setMaxAssetsPerFrame(uint32_t maxAssets) {
		s_maxAssetsPerFrame = maxAssets;
	}

	int AssetManager::getMaxAssetsPerFrame() {
		return s_maxAssetsPerFrame;
	}

	// ----- Mesh Assets -----
	template<>
	AssetHandle<Mesh> AssetManager::load<Mesh>(UUID handle) {
		std::lock_guard<std::recursive_mutex> lock(storage<Mesh>().mutex);

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
		std::lock_guard<std::recursive_mutex> lock(storage<TextureCube>().mutex);

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
		std::lock_guard<std::recursive_mutex> lock(storage<Skybox>().mutex);

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
		std::lock_guard<std::recursive_mutex> lock(storage<Shader>().mutex);

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
		std::lock_guard<std::recursive_mutex> lock(storage<Pipeline>().mutex);

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
		std::lock_guard<std::recursive_mutex> lock(storage<Material>().mutex);

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
		std::lock_guard<std::recursive_mutex> lock(storage<Material>().mutex);

		if (!has<Material>(handle)) {
			AX_CORE_LOG_WARN("Cannot reload material: Handle not found in registry");
			return;
		}

		s_loader->reloadMaterial(handle.uuid, getAssetFilePath(handle));
	}

	// ----- Texture2D Assets -----
	template<>
	AssetHandle<Texture2D> AssetManager::load<Texture2D>(UUID handle) {
		std::lock_guard<std::recursive_mutex> lock(storage<Texture2D>().mutex);

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
		std::lock_guard<std::recursive_mutex> lock(storage<AudioClip>().mutex);

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
		std::lock_guard<std::recursive_mutex> lock(storage<PhysicsMaterial>().mutex);

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
		std::lock_guard<std::recursive_mutex> lock(storage<Prefab>().mutex);

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
		std::lock_guard<std::recursive_mutex> lock(storage<SkeletalMesh>().mutex);

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
		std::lock_guard<std::recursive_mutex> lock(storage<AnimationClip>().mutex);

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
