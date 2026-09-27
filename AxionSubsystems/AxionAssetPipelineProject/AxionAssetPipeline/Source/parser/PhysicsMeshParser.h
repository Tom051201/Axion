#pragma once

#include <string>
#include <filesystem>

#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/physics/PhysicsMesh.h"

namespace Axion::AAP {

	struct PhysicsMeshAssetData {
		UUID uuid;
		std::string name;
		PhysicsMesh::Type type;
		std::filesystem::path sourcePath;
	};

	class PhysicsMeshParser {
	public:

		static void createTextFile(const PhysicsMeshAssetData& data, const std::filesystem::path& outputPath);
		static void createBinaryFile(const PhysicsMeshAssetData& data, const std::filesystem::path& outputPath);

	};

}
