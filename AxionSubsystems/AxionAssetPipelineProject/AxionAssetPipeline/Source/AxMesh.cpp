#include "AxMesh.h"

#include "AxionAssetPipeline/Source/core/BaseIncludes.h"
#include "AxionAssetPipeline/Source/importer/OBJImporter.h"
#include "AxionAssetPipeline/Source/importer/GLTFImporter.h"

namespace Axion::AAP {

	void MeshParser::createTextFile(const MeshAssetData& data, const std::filesystem::path& outputPath) {

		std::filesystem::path absoluteSourcePath = AssetManager::getAbsolute(data.filePath);

		MeshData meshData;
		switch (data.fileFormat) {
			case MeshFormat::GLB: case MeshFormat::GLTF: {
				meshData = GLTFImporter::extractMeshes(absoluteSourcePath);
				break;
			}
			case MeshFormat::OBJ: {
				meshData = OBJImporter::extractMeshes(absoluteSourcePath);
				break;
			}
			default: {
				AX_CORE_LOG_ERROR("Unsupported Mesh Format!");
				return;
			}
		}

		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Version" << YAML::Value << ASSET_VERSION_MESH;
		out << YAML::Key << "Name" << YAML::Value << data.name;
		out << YAML::Key << "UUID" << YAML::Value << data.uuid;
		out << YAML::Key << "Type" << YAML::Value << "Mesh";
		out << YAML::Key << "Format" << YAML::Value << FormatUtils::meshFormatToString(data.fileFormat);
		out << YAML::Key << "Source" << YAML::Value << data.filePath.generic_string();

		out << YAML::Key << "Submeshes" << YAML::Value << YAML::BeginSeq;
		for (const auto& submesh : meshData.submeshes) {
			out << YAML::BeginMap;
			out << YAML::Key << "BaseVertex" << YAML::Value << submesh.baseVertex;
			out << YAML::Key << "StartIndex" << YAML::Value << submesh.startIndex;
			out << YAML::Key << "IndexCount" << YAML::Value << submesh.indexCount;
			out << YAML::Key << "MaterialIndex" << YAML::Value << submesh.materialIndex;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
		AX_CORE_LOG_TRACE("Created .axmesh file ({})", outputPath.string());
	}

	void MeshParser::createBinaryFile(const MeshAssetData& data, const std::filesystem::path& outputPath) {

		std::filesystem::path absoluteSourcePath = AssetManager::getAbsolute(data.filePath);

		MeshData meshData;
		switch (data.fileFormat) {
			case MeshFormat::GLB: case MeshFormat::GLTF: {
				meshData = GLTFImporter::extractMeshes(absoluteSourcePath);
				break;
			}
			case MeshFormat::OBJ: {
				meshData = OBJImporter::extractMeshes(absoluteSourcePath);
				break;
			}
			default: {
				AX_CORE_LOG_ERROR("Unsupported Mesh Format!");
				return;
			}
		}

		// -- Open Binary File --
		std::ofstream out(outputPath, std::ios::out | std::ios::binary);
		if (!out) {
			AX_CORE_LOG_ERROR("Failed to create binary file: {}", outputPath.string());
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
		uint32_t submeshCount = static_cast<uint32_t>(meshData.submeshes.size());
		out.write(reinterpret_cast<const char*>(&vertexCount), sizeof(uint32_t));
		out.write(reinterpret_cast<const char*>(&indexCount), sizeof(uint32_t));
		out.write(reinterpret_cast<const char*>(&submeshCount), sizeof(uint32_t));

		// -- Write Raw Memory --
		out.write(reinterpret_cast<const char*>(meshData.vertices.data()), vertexCount * sizeof(Vertex));
		out.write(reinterpret_cast<const char*>(meshData.indices.data()), indexCount * sizeof(uint32_t));

		if (submeshCount > 0) {
			out.write(reinterpret_cast<const char*>(meshData.submeshes.data()), submeshCount * sizeof(Submesh));
		}

		out.close();
		AX_CORE_LOG_TRACE("Baked binary mesh to {}", outputPath.string());
	}

}
