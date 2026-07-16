#pragma once
#include "axpch.h"

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/AssetHandle.h"
#include "AxionEngine/Source/core/AssetRegistry.h"
#include "AxionEngine/Source/core/AssetLoader.h"
#include "AxionEngine/Source/events/RenderingEvent.h"

namespace Axion {

	class Mesh;
	class Texture2D;
	class TextureCube;
	class Material;
	class Shader;
	class Pipeline;
	class Skybox;
	class AudioClip;
	class PhysicsMaterial;
	class Prefab;
	class SkeletalMesh;
	class AnimationClip;

	template<typename T>
	using AssetMap = std::unordered_map<AssetHandle<T>, Ref<T>>;

	template<typename T>
	using HandleToPathMap = std::unordered_map<AssetHandle<T>, std::filesystem::path>;

	template<typename T>
	using LoadQueue = std::vector<std::pair<AssetHandle<T>, std::function<Ref<T>()>>>;



	// ----- Asset storage template -----
	template<typename T>
	struct AssetStorage {
		AssetMap<T> assets;
		HandleToPathMap<T> handleToPath;
		LoadQueue<T> loadQueue;
		std::recursive_mutex mutex;
	};



	// ----- Asset manager -----
	class AssetManager {
	public:

		AssetManager() = delete;

		static void initialize(AssetLoader* loader);
		static void shutdown();
		static void onEvent(Event& e);

		static std::filesystem::path getRelativeToAssets(const std::filesystem::path& absolutePath);
		static std::filesystem::path getAbsolute(const std::filesystem::path& relativePath);

		static UUID getAssetUUID(const std::filesystem::path& absolutePath);

		template<typename T>
		static AssetHandle<T> load(UUID handle);

		template<typename T>
		static void reload(const AssetHandle<T>& handle);

		static void removeAsset(UUID handle);
		static bool isLoadingAssets();

		static void setMaxAssetsPerFrame(uint32_t maxAssets);
		static int getMaxAssetsPerFrame();

		template<typename T>
		static void submitToMainThread(UUID handle, std::function<Ref<T>()> gpuTask) {
			auto& s = storage<T>();
			std::lock_guard<std::recursive_mutex> lock(s.mutex);
			s.loadQueue.push_back({ handle, gpuTask });
		}

		// -- Templated getter function --
		template<typename T>
		static Ref<T> get(const AssetHandle<T>& handle) {
			auto& s = storage<T>();
			std::lock_guard<std::recursive_mutex> lock(s.mutex);
			auto it = s.assets.find(handle);
			return it != s.assets.end() ? it->second : nullptr;
		}

		// -- Templated has function --
		template<typename T>
		static bool has(const AssetHandle<T>& handle) {
			auto& s = storage<T>();
			std::lock_guard<std::recursive_mutex> lock(s.mutex);
			return s.assets.find(handle) != s.assets.end();
		}

		// -- Templated getMap function --
		template<typename T>
		static const AssetMap<T>& getMap() {
			return storage<T>().assets;
		}

		// -- Templated asset file path function --
		template<typename T>
		static const std::filesystem::path& getAssetFilePath(const AssetHandle<T>& handle) {
			return storage<T>().handleToPath.at(handle);
		}

		template<typename T>
		static AssetStorage<T>& storage() {
			static AssetStorage<T> s_storage;
			return s_storage;
		}

	private:

		inline static AssetLoader* s_loader;

		template<typename T>
		static void release() {
			auto& s = storage<T>();
			s.assets.clear();
			s.handleToPath.clear();
			s.loadQueue.clear();
		}

		template<typename T>
		static void processLoadQueue(uint32_t& maxItems) {
			auto& storageRef = storage<T>();
			std::lock_guard<std::recursive_mutex> lock(storageRef.mutex);

			while (!storageRef.loadQueue.empty() && maxItems > 0) {
				auto it = storageRef.loadQueue.begin();

				storageRef.assets[it->first] = it->second();
				AX_CORE_LOG_TRACE("{} loaded: {}", typeid(T).name(), it->first.uuid.toString());

				storageRef.loadQueue.erase(it);
				maxItems--;
			}
		}

		template<typename T>
		static void removeAssetFromStorage(UUID handle) {
			auto& s = storage<T>();
			std::lock_guard<std::recursive_mutex> lock(s.mutex);
			s.assets.erase(handle);
			s.handleToPath.erase(handle);
		}

		template<typename T>
		static size_t getPendingCount() {
			std::lock_guard<std::recursive_mutex> lock(storage<T>().mutex);
			return storage<T>().loadQueue.size();
		}

	};

	template<> AssetHandle<Mesh> AssetManager::load<Mesh>(UUID handle);
	template<> AssetHandle<Texture2D> AssetManager::load<Texture2D>(UUID handle);
	template<> AssetHandle<TextureCube> AssetManager::load<TextureCube>(UUID handle);
	template<> AssetHandle<Material> AssetManager::load<Material>(UUID handle);
	template<> AssetHandle<Shader> AssetManager::load<Shader>(UUID handle);
	template<> AssetHandle<Pipeline> AssetManager::load<Pipeline>(UUID handle);
	template<> AssetHandle<Skybox> AssetManager::load<Skybox>(UUID handle);
	template<> AssetHandle<AudioClip> AssetManager::load<AudioClip>(UUID handle);
	template<> AssetHandle<PhysicsMaterial> AssetManager::load<PhysicsMaterial>(UUID handle);
	template<> AssetHandle<Prefab> AssetManager::load<Prefab>(UUID handle);
	template<> AssetHandle<SkeletalMesh> AssetManager::load<SkeletalMesh>(UUID handle);
	template<> AssetHandle<AnimationClip> AssetManager::load<AnimationClip>(UUID handle);

}
