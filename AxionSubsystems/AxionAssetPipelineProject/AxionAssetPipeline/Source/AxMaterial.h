#pragma once

#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/render/MaterialData.h"
#include "AxionEngine/Source/render/Texture.h"

#include <string>

namespace Axion::AAP {

	struct MaterialAssetData {
		UUID uuid;
		std::string name;
		MaterialProperties properties;
		std::string pipelineAsset;
		std::map<TextureSlot, std::string> textures;
	};

	class MaterialParser {
	public:

		static void createTextFile(const MaterialAssetData& data, const std::string& outputPath);
		static void createBinaryFile(const MaterialAssetData& data, const std::string& outputPath);

	};

}
