#include "AxPipeline.h"

#include "AxionAssetPipeline/Source/core/BaseIncludes.h"

#include "AxionEngine/Source/core/AssetManager.h"

namespace Axion::AAP {

	void PipelineParser::createTextFile(const PipelineAssetData& data, const std::string& outputPath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Version" << YAML::Value << ASSET_VERSION_PIPELINE;
		out << YAML::Key << "Name" << YAML::Value << data.name;
		out << YAML::Key << "UUID" << YAML::Value << data.uuid.toString();
		out << YAML::Key << "Type" << YAML::Value << "Pipeline";

		out << YAML::Key << "Specification" << YAML::BeginMap;
		UUID shaderUUID = AssetManager::getAssetUUID(AssetManager::getAbsolute(data.shaderFilePath));
		out << YAML::Key << "Shader" << YAML::Value << shaderUUID.toString();
		out << YAML::Key << "NumRenderTargets" << YAML::Value << data.spec.numRenderTargets;
		out << YAML::Key << "ColorFormat" << YAML::Value << EnumUtils::toString(data.spec.colorFormat);
		out << YAML::Key << "DepthStencilFormat" << YAML::Value << EnumUtils::toString(data.spec.depthStencilFormat);
		out << YAML::Key << "DepthTest" << YAML::Value << data.spec.depthTest;
		out << YAML::Key << "DepthWrite" << YAML::Value << data.spec.depthWrite;
		out << YAML::Key << "DepthFunction" << YAML::Value << EnumUtils::toString(data.spec.depthFunction);
		out << YAML::Key << "StencilEnabled" << YAML::Value << data.spec.stencilEnabled;
		out << YAML::Key << "SampleCount" << YAML::Value << data.spec.sampleCount;
		out << YAML::Key << "CullMode" << YAML::Value << EnumUtils::toString(data.spec.cullMode);
		out << YAML::Key << "Topology" << YAML::Value << EnumUtils::toString(data.spec.topology);

		out << YAML::Key << "BufferLayout" << YAML::Value << YAML::BeginSeq;
		for (const auto& elem : data.spec.vertexLayout.getElements()) {
			out << YAML::BeginMap;
			out << YAML::Key << "Name" << YAML::Value << elem.name;
			out << YAML::Key << "Type" << YAML::Value << EnumUtils::toString(elem.type);
			out << YAML::Key << "Size" << YAML::Value << elem.size;
			out << YAML::Key << "Offset" << YAML::Value << elem.offset;
			out << YAML::Key << "Instanced" << YAML::Value << elem.instanced;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		out << YAML::EndMap;

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
		AX_CORE_LOG_TRACE("Created .axpso file ({})", outputPath);
	}

	void PipelineParser::createBinaryFile(const PipelineAssetData& data, const std::string& outputPath) {
		AX_CORE_ASSERT(false, "Creating a binary asset file is not supported yet");
	}

}
