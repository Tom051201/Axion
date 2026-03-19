#include "AxMesh.h"

#include "AxionAssetPipeline/Source/core/BaseIncludes.h"

namespace Axion::AAP {

	void MeshParser::createTextFile(const MeshAssetData& data, const std::string& outputPath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Version" << YAML::Value << ASSET_VERSION_MESH;
		out << YAML::Key << "Name" << YAML::Value << data.name;
		out << YAML::Key << "UUID" << YAML::Value << data.uuid;
		out << YAML::Key << "Type" << YAML::Value << "Mesh";
		out << YAML::Key << "Format" << YAML::Value << data.fileFormat;
		out << YAML::Key << "Source" << YAML::Value << data.filePath;

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
		AX_CORE_LOG_TRACE("Created .axmesh file ({})", outputPath);
	}

	void MeshParser::createBinaryFile(const MeshAssetData& data, const std::string& outputPath) {
		auto meshData = Mesh::loadOBJ(data.filePath);

		// -- Open Binary File --
		std::ofstream out(outputPath, std::ios::out | std::ios::binary);
		if (!out) {
			AX_CORE_LOG_ERROR("Failed to create binary file: {}", outputPath);
			return;
		}

		// -- Write Header --
		BinaryAssetHeader header;
		header.type = AssetType::Mesh;
		header.uuid = data.uuid;
		header.version = ASSET_VERSION_MESH;
		out.write(reinterpret_cast<const char*>(&header), sizeof(BinaryAssetHeader));

		// -- Write Data Sizes --
		uint32_t vertexCount = static_cast<uint32_t>(meshData.vertices.size());
		uint32_t indexCount = static_cast<uint32_t>(meshData.indices.size());
		out.write(reinterpret_cast<const char*>(&vertexCount), sizeof(uint32_t));
		out.write(reinterpret_cast<const char*>(&indexCount), sizeof(uint32_t));

		// -- Write Raw Memory --
		out.write(reinterpret_cast<const char*>(meshData.vertices.data()), vertexCount * sizeof(Vertex));
		out.write(reinterpret_cast<const char*>(meshData.indices.data()), indexCount * sizeof(uint32_t));

		out.close();
		AX_CORE_LOG_TRACE("Baked binary mesh to {}", outputPath);
	}

}
