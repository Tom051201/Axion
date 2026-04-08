#pragma once

#include <filesystem>

#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/render/Shader.h"

namespace Axion::AAP {

	struct ShaderAssetData {
		UUID uuid;
		ShaderSpecification spec;
		std::string fileFormat; // .hlsl
		std::filesystem::path filePath;
	};

	class ShaderParser {
	public:

		static void createTextFile(const ShaderAssetData& data, const std::filesystem::path& outputPath);
		static void createBinaryFile(const ShaderAssetData& data, const std::filesystem::path& outputPath);

	};

}
