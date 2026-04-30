#include "AxSkeletalMesh.h"

#include "AxionEngine/Source/render/SkeletalMesh.h"

#include "AxionAssetPipeline/Source/core/BaseIncludes.h"
#include "AxionAssetPipeline/Source/importer/GLTFImporter.h"

namespace Axion::AAP {

	void SkeletalMeshParser::createTextFile(const SkeletalMeshAssetData& data, const std::filesystem::path& outputPath) {
		SkeletalMeshData meshData = GLTFImporter::extractSkeletalMesh(AssetManager::getAbsolute(data.filePath));

		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Version" << YAML::Value << ASSET_VERSION_SKELETAL_MESH;
		out << YAML::Key << "Name" << YAML::Value << data.name;
		out << YAML::Key << "UUID" << YAML::Value << data.uuid;
		out << YAML::Key << "Type" << YAML::Value << "SkeletalMesh";
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
		AX_CORE_LOG_TRACE("Created .axskelmesh file ({})", outputPath.string());
	}

	void SkeletalMeshParser::createBinaryFile(const SkeletalMeshAssetData& data, const std::filesystem::path& outputPath) {
		SkeletalMeshData meshData = GLTFImporter::extractSkeletalMesh(AssetManager::getAbsolute(data.filePath));

		// -- Open Binary File --
		std::ofstream out(outputPath, std::ios::out | std::ios::binary);
		if (!out) {
			AX_CORE_LOG_ERROR("Failed to create binary file: {}", outputPath.string());
			return;
		}

		// -- Write Header --
		BinaryAssetHeader header;
		header.type = AssetType::SkeletalMesh;
		header.uuid = data.uuid;
		header.version = ASSET_VERSION_SKELETAL_MESH;
		out.write(reinterpret_cast<const char*>(&header), sizeof(BinaryAssetHeader));

		// -- Write Data Sizes --
		uint32_t vertexCount = static_cast<uint32_t>(meshData.vertices.size());
		uint32_t indexCount = static_cast<uint32_t>(meshData.indices.size());
		uint32_t submeshCount = static_cast<uint32_t>(meshData.submeshes.size());
		uint32_t boneCount = static_cast<uint32_t>(meshData.skeleton.bones.size());
		out.write(reinterpret_cast<const char*>(&vertexCount), sizeof(uint32_t));
		out.write(reinterpret_cast<const char*>(&indexCount), sizeof(uint32_t));
		out.write(reinterpret_cast<const char*>(&submeshCount), sizeof(uint32_t));
		out.write(reinterpret_cast<const char*>(&boneCount), sizeof(uint32_t));

		// -- Write Raw Memory --
		out.write(reinterpret_cast<const char*>(meshData.vertices.data()), vertexCount * sizeof(SkeletalVertex));
		out.write(reinterpret_cast<const char*>(meshData.indices.data()), indexCount * sizeof(uint32_t));

		if (submeshCount > 0) {
			out.write(reinterpret_cast<const char*>(meshData.submeshes.data()), submeshCount * sizeof(Submesh));
		}

		// -- Write Bones --
		for (const auto& bone : meshData.skeleton.bones) {
			uint32_t nameLength = static_cast<uint32_t>(bone.name.size());
			out.write(reinterpret_cast<const char*>(&nameLength), sizeof(uint32_t));
			out.write(bone.name.data(), nameLength);

			out.write(reinterpret_cast<const char*>(&bone.parentIndex), sizeof(int32_t));
			out.write(reinterpret_cast<const char*>(&bone.localBindTransform), sizeof(DirectX::XMMATRIX));
			out.write(reinterpret_cast<const char*>(&bone.inverseBindMatrix), sizeof(DirectX::XMMATRIX));
			
			uint32_t childCount = static_cast<uint32_t>(bone.children.size());
			out.write(reinterpret_cast<const char*>(&childCount), sizeof(uint32_t));
			if (childCount > 0) {
				out.write(reinterpret_cast<const char*>(bone.children.data()), childCount * sizeof(int));
			}
		}

		out.write(reinterpret_cast<const char*>(&meshData.skeleton.rootTransform), sizeof(DirectX::XMMATRIX));

		out.close();
		AX_CORE_LOG_TRACE("Baked binary skeletal mesh to {}", outputPath.string());
	}

}
