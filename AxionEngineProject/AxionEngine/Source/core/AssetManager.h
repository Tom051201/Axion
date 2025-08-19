#pragma once
#include "axpch.h"

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/AssetHandle.h"
#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Texture.h"

namespace Axion {

	template<typename T>
	using AssetMap = std::unordered_map<AssetHandle<T>, Ref<T>>;


	class AssetManager {
	public:

		AssetManager() = delete;

		static void initialize();
		static void release();

		static AssetHandle<Mesh> loadMesh(const std::string& path);
		static Ref<Mesh> get(const AssetHandle<Mesh>& handle);
		static const AssetMap<Mesh>& getMeshMap() { return s_meshes; }

	private:

		static AssetMap<Mesh> s_meshes;

	};

}
