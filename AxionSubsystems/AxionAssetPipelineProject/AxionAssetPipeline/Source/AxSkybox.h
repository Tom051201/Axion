#pragma once

#include <string>

#include "AxionEngine/Source/core/UUID.h"

namespace Axion::AAP {

	struct SkyboxAssetData {
		UUID uuid;
		std::string name;
		std::string textureCubePath;
		std::string pipelinePath;
	};

	class SkyboxParser {
	public:

		static void createTextFile(const SkyboxAssetData& data, const std::string& outputPath);
		static void createBinaryFile(const SkyboxAssetData& data, const std::string& outputPath);

	};

}
