#pragma once

#include <string>
#include <filesystem>

#include "AxionEngine/Source/core/UUID.h"

namespace Axion::AAP {

	struct SkeletalMeshAssetData {
		UUID uuid;
		std::string name;
		std::filesystem::path filePath;
	};

	class SkeletalMeshParser {
	public:

		static void createTextFile(const SkeletalMeshAssetData& data, const std::filesystem::path& outputPath);
		static void createBinaryFile(const SkeletalMeshAssetData& data, const std::filesystem::path& outputPath);

	};

}
