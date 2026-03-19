#pragma once

#include "AxionEngine/Source/core/UUID.h"

#include <string>

namespace Axion::AAP {

	struct TextureCubeAssetData {
		UUID uuid;
		std::string name;
		std::string fileFormat;
		std::string filePath;
	};

	class TextureCubeParser {
	public:

		static void createTextFile(const TextureCubeAssetData& data, const std::string& outputPath);
		static void createBinaryFile(const TextureCubeAssetData& data, const std::string& outputPath);

	};

}
