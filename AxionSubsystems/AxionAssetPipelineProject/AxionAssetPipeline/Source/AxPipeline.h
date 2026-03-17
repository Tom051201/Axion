#pragma once

#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/render/Pipeline.h"

namespace Axion::AAP {

	struct PipelineAssetData {
		UUID uuid;
		PipelineSpecification spec;
		std::string name;
		std::string shaderFilePath;
	};

	class PipelineParser {
	public:

		static void createAxPipelineFile(const PipelineAssetData& data, const std::string& outputPath);
		static void createAxPipelineBinary(const PipelineAssetData& data, const std::string& outputPath);

	};

}
