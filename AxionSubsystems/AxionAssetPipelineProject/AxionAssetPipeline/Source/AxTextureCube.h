#pragma once

#include <string>
#include <filesystem>

#include "AxionEngine/Source/core/UUID.h"

#include "AxionAssetPipeline/Source/core/AssetFormats.h"

namespace Axion::AAP {

	struct TextureCubeAssetData {
		UUID uuid;
		std::string name;
		TextureFormat fileFormat = TextureFormat::None;
		std::filesystem::path filePath;
	};

	class TextureCubeParser {
	public:

		static void createTextFile(const TextureCubeAssetData& data, const std::filesystem::path& outputPath);
		static void createBinaryFile(const TextureCubeAssetData& data, const std::filesystem::path& outputPath);

	};

}
