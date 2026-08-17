#pragma once

#include <filesystem>
#include <string>

#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/graphics/Pipeline.h"

namespace Axion::AAP {

	struct PipelineAssetData {
		UUID uuid;
		PipelineSpecification spec;
		std::string name;
		std::filesystem::path shaderFilePath;
	};

	class PipelineParser {
	public:

		static void createTextFile(const PipelineAssetData& data, const std::filesystem::path& outputPath);
		static void createBinaryFile(const PipelineAssetData& data, const std::filesystem::path& outputPath);

	};

}
