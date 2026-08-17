#pragma once

#include <string>
#include <filesystem>
#include <map>

#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/graphics/MaterialData.h"
#include "AxionEngine/Source/graphics/Texture.h"

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
