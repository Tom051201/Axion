#pragma once

#include "AxionEngine/Source/core/AssetHandle.h"

#include <string>

namespace Axion::AAP {

	struct PhysicsMaterialAssetData {
		std::string name;
		float staticFriction;
		float dynamicFriction;
		float restitution;
	};

	class PhysicsMaterialParser {
	public:

		static void createAxPhyMatFile(const PhysicsMaterialAssetData& data, const std::string& outputPath);
		static void createAxPhyMatBinary(const PhysicsMaterialAssetData& data, const std::string& outputPath);

	};

}
