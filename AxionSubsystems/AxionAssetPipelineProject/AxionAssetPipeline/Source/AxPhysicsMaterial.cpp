#include "AxPhysicsMaterial.h"

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/core/YamlHelper.h"

#include <fstream>

namespace Axion::AAP {

	void PhysicsMaterialParser::createAxPhyMatFile(const PhysicsMaterialAssetData& data, const std::string& outputPath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Name" << YAML::Value << data.name;
		out << YAML::Key << "UUID" << YAML::Value << UUID::generate().toString();
		out << YAML::Key << "Type" << YAML::Value << "PhysicsMaterial";

		out << YAML::Key << "StaticFriction" << YAML::Value << data.staticFriction;
		out << YAML::Key << "DynamicFriction" << YAML::Value << data.dynamicFriction;
		out << YAML::Key << "Restitution" << YAML::Value << data.restitution;

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
		AX_CORE_LOG_TRACE("Create .axpmat file ({})", outputPath);
	}

	void PhysicsMaterialParser::createAxPhyMatBinary(const PhysicsMaterialAssetData& data, const std::string& outputPath) {
		AX_CORE_ASSERT(false, "Creating a binary asset file is not supported yet");
	}

}