#pragma once

#include <string>
#include <filesystem>

#include "AxionEngine/Source/core/UUID.h"

namespace Axion::AAP {

	struct SkyboxAssetData {
		UUID uuid;
		std::string name;
		std::filesystem::path textureCubePath;
		std::filesystem::path pipelinePath;
	};

	class SkyboxParser {
	public:

		static void createTextFile(const SkyboxAssetData& data, const std::filesystem::path& outputPath);
		static void createBinaryFile(const SkyboxAssetData& data, const std::filesystem::path& outputPath);

	};

}
