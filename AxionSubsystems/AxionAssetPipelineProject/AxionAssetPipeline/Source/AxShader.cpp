#include "AxShader.h"

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/core/EnumUtils.h"

#include <fstream>

namespace Axion::AAP {

	void ShaderParser::createAxShaderFile(const ShaderAssetData& data, const std::string& outputPath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Name" << YAML::Value << data.spec.name;
		out << YAML::Key << "UUID" << YAML::Value << UUID::generate().toString();
		out << YAML::Key << "Type" << YAML::Value << "Shader";
		out << YAML::Key << "Format" << YAML::Value << data.fileFormat;
		out << YAML::Key << "Source" << YAML::Value << data.filePath;
		
		out << YAML::Key << "Specification" << YAML::BeginMap;
		out << YAML::Key << "Name" << YAML::Value << data.spec.name;
		out << YAML::Key << "BatchTextures" << YAML::Value << data.spec.batchTextures;

		out << YAML::EndMap;

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
		AX_CORE_LOG_TRACE("Created .axshader file ({})", outputPath);
	}

	void ShaderParser::createAxShaderBinary(const ShaderAssetData& data, const std::string& outputPath) {
		AX_CORE_ASSERT(false, "Creating a binary asset file is not supported yet");
	}

}
