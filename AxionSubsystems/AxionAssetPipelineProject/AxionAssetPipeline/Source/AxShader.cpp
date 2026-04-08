#include "AxShader.h"

#include "AxionAssetPipeline/Source/core/BaseIncludes.h"

namespace Axion::AAP {

	void ShaderParser::createTextFile(const ShaderAssetData& data, const std::filesystem::path& outputPath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Version" << YAML::Value << ASSET_VERSION_SHADER;
		out << YAML::Key << "Name" << YAML::Value << data.spec.name;
		out << YAML::Key << "UUID" << YAML::Value << data.uuid.toString();
		out << YAML::Key << "Type" << YAML::Value << "Shader";
		out << YAML::Key << "Format" << YAML::Value << data.fileFormat;
		out << YAML::Key << "Source" << YAML::Value << data.filePath.generic_string();
		
		out << YAML::Key << "Specification" << YAML::BeginMap;
		out << YAML::Key << "Name" << YAML::Value << data.spec.name;
		out << YAML::Key << "BatchTextures" << YAML::Value << data.spec.batchTextures;

		out << YAML::EndMap;

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
		AX_CORE_LOG_TRACE("Created .axshader file ({})", outputPath.string());
	}

	void ShaderParser::createBinaryFile(const ShaderAssetData& data, const std::filesystem::path& outputPath) {
		ShaderBytecode bytecode = Shader::compileToBytecode(data.filePath);

		if (!bytecode.isValid()) {
			AX_CORE_LOG_ERROR("Failed to compile shader for packaging: {}", data.filePath.string());
			return;
		}

		std::ofstream out(outputPath, std::ios::out | std::ios::binary);
		if (!out) {
			AX_CORE_LOG_ERROR("Failed to create binary file: {}", outputPath.string());
			return;
		}

		// -- Write Header --
		ShaderBinaryHeader header = {};
		header.assetHeader.type = AssetType::Shader;
		header.assetHeader.uuid = data.uuid;
		header.assetHeader.version = ASSET_VERSION_SHADER;
		header.batchTextures = data.spec.batchTextures;
		header.vsSize = static_cast<uint64_t>(bytecode.vertex.size());
		header.psSize = static_cast<uint64_t>(bytecode.pixel.size());
		out.write(reinterpret_cast<const char*>(&header), sizeof(ShaderBinaryHeader));

		// -- Write Bytecode --
		out.write(reinterpret_cast<const char*>(bytecode.vertex.data()), header.vsSize);
		out.write(reinterpret_cast<const char*>(bytecode.pixel.data()), header.psSize);

		out.close();
		AX_CORE_LOG_TRACE("Baked binary Shader to {}", outputPath.string());
	}

}
