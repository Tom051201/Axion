#pragma once

#include <string>
#include <filesystem>

#include "AxionEngine/Source/core/UUID.h"

namespace Axion::AAP {

	struct Texture2DAssetData {
		UUID uuid;
		std::string name;
		std::string fileFormat;
		std::filesystem::path filePath;
	};

	class Texture2DParser {
	public:

		static void createTextFile(const Texture2DAssetData& data, const std::filesystem::path& outputPath);
		static void createBinaryFile(const Texture2DAssetData& data, const std::filesystem::path& outputPath);

	};

}
