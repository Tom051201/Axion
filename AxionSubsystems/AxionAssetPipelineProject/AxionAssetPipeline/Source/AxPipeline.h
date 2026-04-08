#pragma once

#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/render/Pipeline.h"

#include <filesystem>

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
