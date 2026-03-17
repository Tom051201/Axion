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

		static void createAxMeshFile(const MeshAssetData& data, const std::string& outputPath);
		static void createAxMeshBinary(const MeshAssetData& data, const std::string& outputPath);

	};

}
