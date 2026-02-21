#pragma once

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/core/AssetHandle.h"
#include "AxionEngine/Source/render/MaterialData.h"
#include "AxionEngine/Source/render/Texture.h"

#include <string>

namespace Axion::AAP {

	struct MaterialAssetData {
		std::string name;
		MaterialProperties properties;
		std::string pipelineAsset;
		std::map<TextureSlot, std::string> textures;
	};

	class MaterialParser {
	public:

		static void createAxMatFile(const MaterialAssetData& data, const std::string& outputPath);
		static void createAxMatBinary(const MaterialAssetData& data, const std::string& outputPath);

	};

}
