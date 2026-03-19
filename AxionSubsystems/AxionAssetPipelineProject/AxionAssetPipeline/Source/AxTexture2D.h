#pragma once

#include "AxionEngine/Source/core/UUID.h"

#include <string>

namespace Axion::AAP {

	struct Texture2DAssetData {
		UUID uuid;
		std::string name;
		std::string fileFormat;
		std::string filePath;
	};

	class Texture2DParser {
	public:

		static void createTextFile(const Texture2DAssetData& data, const std::string& outputPath);
		static void createBinaryFile(const Texture2DAssetData& data, const std::string& outputPath);

	};

}
