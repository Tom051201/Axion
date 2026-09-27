#include "PhysicsMeshParser.h"
#include "AxionAssetPipeline/Source/core/BaseIncludes.h"

namespace Axion::AAP {

	void PhysicsMeshParser::createTextFile(const PhysicsMeshAssetData& data, const std::filesystem::path& outputPath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Version" << YAML::Value << ASSET_VERSION_PHYSICS_MESH;
		out << YAML::Key << "Name" << YAML::Value << data.name;
		out << YAML::Key << "UUID" << YAML::Value << data.uuid;
		out << YAML::Key << "Type" << YAML::Value << "PhysicsMesh";

		std::string typeStr = (data.type == PhysicsMesh::Type::Convex) ? "Convex" : "Triangle";
		out << YAML::Key << "PhysicsType" << YAML::Value << typeStr;
		out << YAML::Key << "Source" << YAML::Value << data.sourcePath.generic_string();

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
		AX_CORE_LOG_TRACE("Create .axpmesh file ({})", outputPath.string());
	}

	void PhysicsMeshParser::createBinaryFile(const PhysicsMeshAssetData& data, const std::filesystem::path& outputPath) {
		std::ifstream in(data.sourcePath, std::ios::in | std::ios::binary);
		if (!in.is_open()) {
			AX_CORE_LOG_ERROR("Failed to open source file: {}", data.sourcePath.string());
			return;
		}

		in.seekg(0, std::ios::end);
		size_t size = in.tellg();
		in.seekg(0, std::ios::beg);

		std::vector<char> buffer(size);
		in.read(buffer.data(), size);
		in.close();

		std::ofstream out(outputPath, std::ios::out | std::ios::binary);
		if (!out) {
			AX_CORE_LOG_ERROR("Failed to create binary file: {}", outputPath.string());
			return;
		}

		// -- Write Header --
		BinaryAssetHeader header;
		header.type = AssetType::PhysicsMesh;
		header.uuid = data.uuid;
		header.version = ASSET_VERSION_PHYSICS_MESH;
		out.write(reinterpret_cast<const char*>(&header), sizeof(BinaryAssetHeader));

		// -- Write Data --
		uint32_t pType = static_cast<uint32_t>(data.type);
		out.write(reinterpret_cast<const char*>(&pType), sizeof(uint32_t));
		out.write(buffer.data(), size);

		out.close();
		AX_CORE_LOG_TRACE("Baked binary PhysicsMesh to {}", outputPath.string());
	}

}
