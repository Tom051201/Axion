#pragma once

#include <string>
#include <unordered_map>

#include "AxionEngine/Source/core/UUID.h"
#include "AxionAssetPipeline/Source/core/ParsingUtils.h"

namespace Axion::AAP {

	struct SkyboxAssetData {
		UUID uuid;
		std::string name;
		bool singleFileImport = true;
		std::string singleFilePath;
		std::unordered_map<std::string, std::string> facesFilePaths;
		std::string pipelinePath;
	};

	class SkyboxParser {
	public:

		static void createAxSkyFile(const SkyboxAssetData& data, const std::string& outputPath);
		static void createAxSkyBinary(const SkyboxAssetData& data, const std::string& outputPath);

	};

}
