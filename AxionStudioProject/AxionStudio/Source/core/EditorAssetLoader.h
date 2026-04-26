#pragma once

#include "AxionEngine/Source/core/AssetLoader.h"

namespace Axion {

	class EditorAssetLoader : public AssetLoader {
	public:

		UUID peekUUID(const std::filesystem::path& absolutePath) override;

		void loadMesh(UUID handle, const std::filesystem::path& absolutePath) override;
		void loadTexture2D(UUID handle, const std::filesystem::path& absolutePath) override;
		void loadTextureCube(UUID handle, const std::filesystem::path& absolutePath) override;
		void loadSkybox(UUID handle, const std::filesystem::path& absolutePath) override;
		void loadShader(UUID handle, const std::filesystem::path& absolutePath) override;
		void loadPipeline(UUID handle, const std::filesystem::path& absolutePath) override;
		void loadMaterial(UUID handle, const std::filesystem::path& absolutePath) override;
		void loadAudioClip(UUID handle, const std::filesystem::path& absolutePath) override;
		void loadPhysicsMaterial(UUID handle, const std::filesystem::path& absolutePath) override;
		void loadPrefab(UUID handle, const std::filesystem::path& absolutePath) override;
		void loadAnimationClip(UUID handle, const std::filesystem::path& absolutePath) override;
		void loadSkeletalMesh(UUID handle, const std::filesystem::path& absolutePath) override;

		void reloadMaterial(UUID handle, const std::filesystem::path& absolutePath) override;

	};
}
