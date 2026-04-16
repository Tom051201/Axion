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

		// -- Templated getter function --
		template<typename T>
		static Ref<T> get(const AssetHandle<T>& handle) {
			auto& map = storage<T>().assets;
			auto it = map.find(handle);
			return it != map.end() ? it->second : nullptr;
		}

		// -- Templated has function --
		template<typename T>
		static bool has(const AssetHandle<T>& handle) {
			return storage<T>().assets.find(handle) != storage<T>().assets.end();
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
		static void processLoadQueue() {
			auto& storageRef = storage<T>();
			for (auto& [handle, task] : storageRef.loadQueue) {
				storageRef.assets[handle] = task();
				AX_CORE_LOG_TRACE("{} loaded: {}", typeid(T).name(), handle.uuid.toString());
			}
			storageRef.loadQueue.clear();
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

}
