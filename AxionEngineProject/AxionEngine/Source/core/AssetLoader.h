#pragma once

#include <filesystem>

#include "AxionEngine/Source/core/UUID.h"

namespace Axion {

	class AssetLoader {
	public:

		virtual ~AssetLoader() = default;

		virtual UUID peekUUID(const std::filesystem::path& absolutePath) = 0;

		virtual void loadMesh(UUID handle, const std::filesystem::path& absolutePath) = 0;
		virtual void loadTexture2D(UUID handle, const std::filesystem::path& absolutePath) = 0;
		virtual void loadTextureCube(UUID handle, const std::filesystem::path& absolutePath) = 0;
		virtual void loadSkybox(UUID handle, const std::filesystem::path& absolutePath) = 0;
		virtual void loadShader(UUID handle, const std::filesystem::path& absolutePath) = 0;
		virtual void loadPipeline(UUID handle, const std::filesystem::path& absolutePath) = 0;
		virtual void loadMaterial(UUID handle, const std::filesystem::path& absolutePath) = 0;
		virtual void loadAudioClip(UUID handle, const std::filesystem::path& absolutePath) = 0;
		virtual void loadPhysicsMaterial(UUID handle, const std::filesystem::path& absolutePath) = 0;
		virtual void loadPrefab(UUID handle, const std::filesystem::path& absolutePath) = 0;

		virtual void reloadMaterial(UUID handle, const std::filesystem::path& absolutePath) = 0;

	};

}
