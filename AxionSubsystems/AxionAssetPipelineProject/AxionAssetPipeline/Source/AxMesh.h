#pragma once

#include "AxionEngine/Source/core/UUID.h"

#include <string>

namespace Axion::AAP {

	struct MeshAssetData {
		UUID uuid;
		std::string name;
		std::string fileFormat; // OBJ
		std::string filePath;
	};

	class MeshParser {
	public:

		static void createTextFile(const MeshAssetData& data, const std::string& outputPath);
		static void createBinaryFile(const MeshAssetData& data, const std::string& outputPath);

	};

}
