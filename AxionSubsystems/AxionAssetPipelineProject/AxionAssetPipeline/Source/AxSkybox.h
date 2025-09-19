#pragma once

#include <string>
#include <unordered_map>

#include "AxionAssetPipeline/Source/core/ParsingUtils.h"

namespace Axion::AAP {

	struct SkyboxAssetData {
		std::string name;
		bool singleFileImport = true;
		std::string singleFilePath;
		std::unordered_map<std::string, std::string> facesFilePaths;
	};

	class SkyboxParser {
	public:

		static void createAxSkyFile(const SkyboxAssetData& data, const std::string& outputPath);
		static void createAxSkyBinary(const SkyboxAssetData& data, const std::string& outputPath);

	};

}
