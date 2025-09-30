#pragma once
#include "axpch.h"

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/AssetHandle.h"
#include "AxionEngine/Source/events/RenderingEvent.h"

namespace Axion {

	// ----- Usings for easier syntax -----
	template<typename T>
	using AssetMap = std::unordered_map<AssetHandle<T>, Ref<T>>;

	// Stores the absolute path to the asset file (for example *.axsky)
	template<typename T>
	using HandleToPathMap = std::unordered_map<AssetHandle<T>, std::string>;

	template<typename T>
	using LoadQueue = std::vector<std::pair<AssetHandle<T>, std::string>>;



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

		static void initialize();
		static void shutdown();
		static void onEvent(Event& e);

		static std::string getRelativeToAssets(const std::string& absolutePath);	// Returns the relative path to the Assets directory
		static std::filesystem::path getRelativeToAssets(const std::filesystem::path& absolutePath);
		static std::string getAbsolute(const std::string& relativePath);			// Returns the absolute path

		template<typename T>
		static AssetHandle<T> load(const std::string& absolutePath);

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
		static const std::string& getAssetFilePath(const AssetHandle<T>& handle) {
			return storage<T>().handleToPath.at(handle);
		}

	private:

		template<typename T>
		static AssetStorage<T>& storage() {
			static AssetStorage<T> s_storage;
			return s_storage;
		}

		template<typename T>
		static void release() {
			auto& s = storage<T>();
			s.assets.clear();
			s.handleToPath.clear();
			s.loadQueue.clear();
		}

		template<typename T, typename Loader>
		static void processLoadQueue(Loader loader) {
			auto& storageRef = storage<T>();
			for (auto& [handle, source] : storageRef.loadQueue) {
				Ref<T> asset = loader(source, handle);
				storageRef.assets[handle] = asset;
				AX_CORE_LOG_TRACE("{} loaded: {}", typeid(T).name(), handle.uuid.toString());
			}
			storageRef.loadQueue.clear();
		}

	};

}
