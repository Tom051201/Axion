#pragma once

#include "AxionEngine/Source/core/UUID.h"

#include "AxionAssetPipeline/Source/core/AssetFormats.h"

#include <string>
#include <filesystem>

namespace Axion::AAP {

	struct MeshAssetData {
		UUID uuid;
		std::string name;
		MeshFormat fileFormat = MeshFormat::None;
		std::filesystem::path filePath;
	};

	class MeshParser {
	public:

		static void createTextFile(const MeshAssetData& data, const std::filesystem::path& outputPath);
		static void createBinaryFile(const MeshAssetData& data, const std::filesystem::path& outputPath);

	};

}
