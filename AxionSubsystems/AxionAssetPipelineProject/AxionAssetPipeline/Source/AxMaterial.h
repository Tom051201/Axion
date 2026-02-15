#pragma once

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/render/MaterialData.h"

#include <string>

namespace Axion::AAP {

	struct MaterialAssetData {
		std::string name;
		MaterialProperties properties;
		std::string shaderAsset;
	};

	class MaterialParser {
	public:

		static void createAxMatFile(const MaterialAssetData& data, const std::string& outputPath);
		static void createAxMatBinary(const MaterialAssetData& data, const std::string& outputPath);

	};

}
