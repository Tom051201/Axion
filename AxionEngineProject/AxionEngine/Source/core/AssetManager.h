#pragma once
#include "axpch.h"

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/AssetHandle.h"
#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Texture.h"
#include "AxionEngine/Source/events/RenderingEvent.h"
#include "AxionEngine/Source/scene/Skybox.h"

namespace Axion {

	template<typename T>
	using AssetMap = std::unordered_map<AssetHandle<T>, Ref<T>>;

	// Stores the absolute path to the asset file (for example *.axsky)
	template<typename T>
	using HandleToPathMap = std::unordered_map<AssetHandle<T>, std::string>;

	template<typename T>
	using LoadQueue = std::vector<std::pair<AssetHandle<T>, std::string>>;

	class AssetManager {
	public:

		AssetManager() = delete;

		static void initialize();
		static void release();
		static void onEvent(Event& e);

		static std::string getRelativeToAssets(const std::string& absolutePath);	// Returns the relative path to the Assets directory
		static std::string getAbsolute(const std::string& relativePath);			// Returns the absolute path

		static AssetHandle<Mesh> loadMesh(const std::string& absolutePath);
		static Ref<Mesh> getMesh(const AssetHandle<Mesh>& handle);
		static const AssetMap<Mesh>& getMeshMap() { return s_meshes; }
		static bool hasMesh(const AssetHandle<Mesh>& handle);
		static const std::string& getMeshAssetFilePath(const AssetHandle<Mesh>& handle);

		static AssetHandle<Skybox> loadSkybox(const std::string& absolutePath);
		static Ref<Skybox> getSkybox(const AssetHandle<Skybox>& handle);
		static const AssetMap<Skybox>& getSkyboxMap() { return s_skyboxes; }
		static bool hasSkybox(const AssetHandle<Skybox>& handle);
		static const std::string& getSkyboxAssetFilePath(const AssetHandle<Skybox>& handle);

	private:

		static AssetMap<Mesh> s_meshes;
		static HandleToPathMap<Mesh> s_meshHandleToPath;

		static AssetMap<Skybox> s_skyboxes;
		static HandleToPathMap<Skybox> s_skyboxHandleToPath;
		static LoadQueue<Skybox> s_skyboxLoadQueue;

	};

}
