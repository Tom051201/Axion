#pragma once

#include "AxionEngine/Source/core/UUID.h"

#include <string>
#include <filesystem>

namespace Axion::AAP {

	struct AnimationClipAssetData {
		UUID uuid;
		std::string name;
		std::filesystem::path filePath;
	};

	class AnimationClipParser {
	public:

		static void createTextFile(const AnimationClipAssetData& data, const std::filesystem::path& outputPath);
		static void createBinaryFile(const AnimationClipAssetData& data, const std::filesystem::path& outputPath);

	};

}
