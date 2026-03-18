#include "AxPhysicsMaterial.h"

#include "AxionAssetPipeline/Source/core/BaseIncludes.h"

namespace Axion::AAP {

	void PhysicsMaterialParser::createTextFile(const PhysicsMaterialAssetData& data, const std::string& outputPath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Version" << YAML::Value << ASSET_VERSION_PHYSICS_MATERIAL;
		out << YAML::Key << "Name" << YAML::Value << data.name;
		out << YAML::Key << "UUID" << YAML::Value << data.uuid;
		out << YAML::Key << "Type" << YAML::Value << "PhysicsMaterial";

		out << YAML::Key << "StaticFriction" << YAML::Value << data.staticFriction;
		out << YAML::Key << "DynamicFriction" << YAML::Value << data.dynamicFriction;
		out << YAML::Key << "Restitution" << YAML::Value << data.restitution;

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
		AX_CORE_LOG_TRACE("Create .axpmat file ({})", outputPath);
	}

	void PhysicsMaterialParser::createBinaryFile(const PhysicsMaterialAssetData& data, const std::string& outputPath) {
		AX_CORE_ASSERT(false, "Creating a binary asset file is not supported yet");
	}

}