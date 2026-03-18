#include "AxPrefab.h"

#include "AxionAssetPipeline/Source/core/BaseIncludes.h"

#include "AxionEngine/Source/scene/SceneSerializer.h"

namespace Axion::AAP {

	void PrefabParser::createTextFile(const PrefabAssetData& data, const std::string& outputPath) {
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
		AX_CORE_LOG_TRACE("Created .axprefab file ({})", outputPath);
	}

	void PrefabParser::createBinaryFile(const PrefabAssetData& data, const std::string& outputPath) {
		AX_CORE_ASSERT(false, "Creating a binary asset file is not supported yet");
	}

}
