#include "AxPrefab.h"

#include "AxionAssetPipeline/Source/core/BaseIncludes.h"

#include "AxionEngine/Source/scene/SceneSerializer.h"

namespace Axion::AAP {

	void PrefabParser::createTextFile(const PrefabAssetData& data, const std::filesystem::path& outputPath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Version" << YAML::Value << ASSET_VERSION_PREFAB;
		out << YAML::Key << "Type" << YAML::Value << "Prefab";
		out << YAML::Key << "UUID" << YAML::Value << data.uuid.toString();

		out << YAML::Key << "Entity";
		SceneSerializer serializer(data.scene);
		serializer.serializeEntity(out, data.entity);
		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
		AX_CORE_LOG_TRACE("Created .axprefab file ({})", outputPath.string());
	}

	void PrefabParser::createBinaryFile(const PrefabAssetData& data, const std::filesystem::path& outputPath) {
		std::ofstream out(outputPath, std::ios::out | std::ios::binary);
		if (!out) {
			AX_CORE_LOG_ERROR("Failed to create binary file: {}", outputPath.string());
			return;
		}

		// -- Write Header --
		BinaryAssetHeader header;
		header.type = AssetType::Prefab;
		header.uuid = data.uuid;
		header.version = ASSET_VERSION_PREFAB;
		out.write(reinterpret_cast<const char*>(&header), sizeof(BinaryAssetHeader));

		// -- Write Entity Data --
		SceneSerializer serializer(data.scene);
		serializer.serializeEntityBinary(out, data.entity);

		out.close();
		AX_CORE_LOG_TRACE("Baked binary Prefab to {}", outputPath.string());
	}

}
