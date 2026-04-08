#include "AxPipeline.h"

#include "AxionAssetPipeline/Source/core/BaseIncludes.h"

namespace Axion::AAP {

	void PipelineParser::createTextFile(const PipelineAssetData& data, const std::filesystem::path& outputPath) {
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
			out << YAML::Key << "Normalized" << YAML::Value << elem.normalized;
			out << YAML::Key << "Instanced" << YAML::Value << elem.instanced;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		out << YAML::EndMap;

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
		AX_CORE_LOG_TRACE("Created .axpso file ({})", outputPath.string());
	}

	void PipelineParser::createBinaryFile(const PipelineAssetData& data, const std::filesystem::path& outputPath) {
		std::ofstream out(outputPath, std::ios::out | std::ios::binary);
		if (!out) {
			AX_CORE_LOG_ERROR("Failed to create binary file: {}", outputPath.string());
			return;
		}

		// -- Write Header --
		PipelineBinaryHeader header = {};
		header.assetHeader.type = AssetType::Pipeline;
		header.assetHeader.uuid = data.uuid;
		header.assetHeader.version = ASSET_VERSION_PIPELINE;
		header.shaderUUID = AssetManager::getAssetUUID(AssetManager::getAbsolute(data.shaderFilePath));
		header.colorFormat = static_cast<uint32_t>(data.spec.colorFormat);
		header.depthStencilFormat = static_cast<uint32_t>(data.spec.depthStencilFormat);
		header.depthTest = data.spec.depthTest ? 1 : 0;
		header.depthWrite = data.spec.depthWrite ? 1 : 0;
		header.depthFunction = static_cast<uint32_t>(data.spec.depthFunction);
		header.stencilEnabled = data.spec.stencilEnabled ? 1 : 0;
		header.sampleCount = data.spec.sampleCount;
		header.cullMode = static_cast<uint32_t>(data.spec.cullMode);
		header.topology = static_cast<uint32_t>(data.spec.topology);
		header.numRenderTargets = data.spec.numRenderTargets;
		const auto& elements = data.spec.vertexLayout.getElements();
		header.bufferElementCount = static_cast<uint32_t>(elements.size());
		out.write(reinterpret_cast<const char*>(&header), sizeof(PipelineBinaryHeader));

		// -- Write Buffer Layout Elements --
		for (const auto& elem : elements) {
			uint32_t nameLen = static_cast<uint32_t>(elem.name.size());
			out.write(reinterpret_cast<const char*>(&nameLen), sizeof(uint32_t));
			out.write(elem.name.data(), nameLen);

			uint32_t type = static_cast<uint32_t>(elem.type);
			uint8_t instanced = elem.instanced ? 1 : 0;
			uint8_t normalized = elem.normalized ? 1 : 0;
			out.write(reinterpret_cast<const char*>(&type), sizeof(uint32_t));
			out.write(reinterpret_cast<const char*>(&elem.size), sizeof(uint32_t));
			out.write(reinterpret_cast<const char*>(&elem.offset), sizeof(uint32_t));
			out.write(reinterpret_cast<const char*>(&normalized), sizeof(uint8_t));
			out.write(reinterpret_cast<const char*>(&instanced), sizeof(uint8_t));
		}

		out.close();
		AX_CORE_LOG_TRACE("Baked binary Pipeline to {}", outputPath.string());
	}

}
