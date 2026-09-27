#pragma once

#include <string>
#include <filesystem>

#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/pathfinding/NavMeshSystem.h"

namespace Axion::AAP {

	struct NavMeshAssetData {
		UUID uuid;
		std::string name;
		std::filesystem::path sourcePath;
		NavMeshBuildSettings settings;
	};

	class NavMeshParser {
	public:

		static void createTextFile(const NavMeshAssetData& data, const std::filesystem::path& outputPath);
		static void createBinaryFile(const NavMeshAssetData& data, const std::filesystem::path& outputPath);

	};
}
