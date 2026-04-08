#pragma once

#include <string>
#include <filesystem>

#include "AxionEngine/Source/core/UUID.h"

namespace Axion::AAP {

	struct TextureCubeAssetData {
		UUID uuid;
		std::string name;
		std::string fileFormat;
		std::filesystem::path filePath;
	};

	class TextureCubeParser {
	public:

		static void createTextFile(const TextureCubeAssetData& data, const std::filesystem::path& outputPath);
		static void createBinaryFile(const TextureCubeAssetData& data, const std::filesystem::path& outputPath);

	};

}
