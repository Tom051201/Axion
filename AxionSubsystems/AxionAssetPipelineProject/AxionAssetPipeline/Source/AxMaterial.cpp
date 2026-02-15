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

		out << YAML::Key << "AlbedoColor" << YAML::Value << data.properties.albedoColor;
		out << YAML::Key << "Metalness" << YAML::Value << data.properties.metalness;
		out << YAML::Key << "Roughness" << YAML::Value << data.properties.roughness;
		out << YAML::Key << "Emission" << YAML::Value << data.properties.emissionStrength;
		out << YAML::Key << "Tiling" << YAML::Value << data.properties.tiling;

		out << YAML::Key << "UseNormalMap" << YAML::Value << data.properties.useNormalMap;
		out << YAML::Key << "UseMetalnessMap" << YAML::Value << data.properties.useMetalnessMap;
		out << YAML::Key << "UseRoughnessMap" << YAML::Value << data.properties.useRoughnessMap;
		out << YAML::Key << "UseOcclusionMap" << YAML::Value << data.properties.useOcclusionMap;

		out << YAML::Key << "Shader" << YAML::Value << data.shaderAsset;

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
		AX_CORE_LOG_TRACE("Created .axmat file ({})", outputPath);
	}

	void MaterialParser::createAxMatBinary(const MaterialAssetData& data, const std::string& outputPath) {
		AX_CORE_ASSERT(false, "Creating a binary asset file is not supported yet");
	}

}
