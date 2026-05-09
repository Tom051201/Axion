#pragma once

#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/render/MaterialData.h"
#include "AxionEngine/Source/render/Texture.h"

#include <string>
#include <filesystem>

namespace Axion::AAP {

	struct MaterialAssetData {
		UUID uuid;
		std::string name;
		MaterialProperties properties;
		std::filesystem::path pipelineAsset;
		std::map<TextureSlot, std::filesystem::path> textures;
	};

	class MaterialParser {
	public:

		static void createTextFile(const MaterialAssetData& data, const std::filesystem::path& outputPath);
		static void createBinaryFile(const MaterialAssetData& data, const std::filesystem::path& outputPath);

	};

}
