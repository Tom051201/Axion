#pragma once

#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/render/Shader.h"

namespace Axion::AAP {

	struct ShaderAssetData {
		UUID uuid;
		ShaderSpecification spec;
		std::string fileFormat; // .hlsl
		std::string filePath;
	};

	class ShaderParser {
	public:

		static void createAxShaderFile(const ShaderAssetData& data, const std::string& outputPath);
		static void createAxShaderBinary(const ShaderAssetData& data, const std::string& outputPath);

	};

}
