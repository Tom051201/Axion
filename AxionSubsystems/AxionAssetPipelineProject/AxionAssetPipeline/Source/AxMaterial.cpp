#include "AxMaterial.h"

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/core/YamlHelper.h"

#include <fstream>

namespace Axion::AAP {

	void MaterialParser::createAxMatFile(const MaterialAssetData& data, const std::string& outputPath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Name" << YAML::Value << data.name;
		out << YAML::Key << "UUID" << YAML::Value << UUID::generate().toString();
		out << YAML::Key << "Type" << YAML::Value << "Material";

		out << YAML::Key << "Color" << YAML::Value << data.color;
		out << YAML::Key << "Shader" << YAML::Value << data.shaderAsset;

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
	}

	void MaterialParser::createAxMatBinary(const MaterialAssetData& data, const std::string& outputPath) {
		AX_CORE_ASSERT(false, "Creating a binary asset file is not supported yet");
	}

}
