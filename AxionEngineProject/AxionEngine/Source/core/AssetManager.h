#pragma once
#include "axpch.h"

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/AssetHandle.h"
#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Texture.h"
#include "AxionEngine/Source/scene/Skybox.h"

namespace Axion {

	template<typename T>
	using AssetMap = std::unordered_map<AssetHandle<T>, Ref<T>>;


	class AssetManager {
	public:

		AssetManager() = delete;

		static void initialize();
		static void release();

		static AssetHandle<Mesh> loadMesh(const std::string& path);
		static Ref<Mesh> getMesh(const AssetHandle<Mesh>& handle);
		static const AssetMap<Mesh>& getMeshMap() { return s_meshes; }
		static bool hasMesh(const AssetHandle<Mesh>& handle);

		static AssetHandle<Skybox> loadSkybox(const std::string& path);
		static Ref<Skybox> getSkybox(const AssetHandle<Skybox>& handle);
		static const AssetMap<Skybox>& getSkyboxMap() { return s_skyboxes; }
		static bool hasSkybox(const AssetHandle<Skybox>& handle);

	private:

		static AssetMap<Mesh> s_meshes;
		static AssetMap<Skybox> s_skyboxes;

	};

}
