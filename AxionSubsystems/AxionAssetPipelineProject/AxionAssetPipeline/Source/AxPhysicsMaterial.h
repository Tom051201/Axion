#pragma once

#include "AxionEngine/Source/core/UUID.h"

#include <string>

namespace Axion::AAP {

	struct PhysicsMaterialAssetData {
		UUID uuid;
		std::string name;
		float staticFriction;
		float dynamicFriction;
		float restitution;
	};

	class PhysicsMaterialParser {
	public:

		static void createTextFile(const PhysicsMaterialAssetData& data, const std::string& outputPath);
		static void createBinaryFile(const PhysicsMaterialAssetData& data, const std::string& outputPath);

	};

}
