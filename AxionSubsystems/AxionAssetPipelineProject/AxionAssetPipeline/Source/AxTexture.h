#pragma once

#include <string>

namespace Axion::AAP {

	struct Texture2DAssetData {
		std::string name;
		std::string fileFormat;
		std::string filePath;
	};

	class Texture2DParser {
	public:

		static void createAxTexFile(const Texture2DAssetData& data, const std::string& outputPath);
		static void createAxTexBinary(const Texture2DAssetData& data, const std::string& outputPath);

	};

}
