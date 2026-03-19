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
		std::ofstream out(outputPath, std::ios::out | std::ios::binary);
		if (!out) {
			AX_CORE_LOG_ERROR("Failed to create binary file: {}", outputPath);
			return;
		}

		// -- Write Header --
		BinaryAssetHeader header;
		header.type = AssetType::PhysicsMaterial;
		header.uuid = data.uuid;
		header.version = ASSET_VERSION_PHYSICS_MATERIAL;
		out.write(reinterpret_cast<const char*>(&header), sizeof(BinaryAssetHeader));

		// -- Write Data --
		out.write(reinterpret_cast<const char*>(&data.staticFriction), sizeof(float));
		out.write(reinterpret_cast<const char*>(&data.dynamicFriction), sizeof(float));
		out.write(reinterpret_cast<const char*>(&data.restitution), sizeof(float));

		out.close();
		AX_CORE_LOG_TRACE("Baked binary PhysicsMaterial to {}", outputPath);
	}

}