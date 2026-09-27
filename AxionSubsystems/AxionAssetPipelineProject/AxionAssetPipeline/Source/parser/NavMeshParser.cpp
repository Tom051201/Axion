#include "NavMeshParser.h"

#include "AxionAssetPipeline/Source/core/BaseIncludes.h"

#include "AxionEngine/Source/core/PathResolver.h"

namespace Axion::AAP {

	void NavMeshParser::createTextFile(const NavMeshAssetData& data, const std::filesystem::path& outputPath) {
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Version" << YAML::Value << ASSET_VERSION_NAVMESH;
		out << YAML::Key << "Name" << YAML::Value << data.name;
		out << YAML::Key << "UUID" << YAML::Value << data.uuid;
		out << YAML::Key << "Type" << YAML::Value << "NavMesh";
		out << YAML::Key << "Source" << YAML::Value << PathResolver::virtualize(AssetManager::getAbsolute(data.sourcePath));

		out << YAML::Key << "BuildSettings" << YAML::BeginMap;
		out << YAML::Key << "AgentHeight" << YAML::Value << data.settings.agentHeight;
		out << YAML::Key << "AgentRadius" << YAML::Value << data.settings.agentRadius;
		out << YAML::Key << "AgentMaxClimb" << YAML::Value << data.settings.agentMaxClimb;
		out << YAML::Key << "AgentMaxSlope" << YAML::Value << data.settings.agentMaxSlope;
		out << YAML::Key << "CellSize" << YAML::Value << data.settings.cellSize;
		out << YAML::Key << "CellHeight" << YAML::Value << data.settings.cellHeight;
		out << YAML::EndMap;

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
	}

	void NavMeshParser::createBinaryFile(const NavMeshAssetData& data, const std::filesystem::path& outputPath) {
		std::filesystem::path absoluteSourcePath = AssetManager::getAbsolute(data.sourcePath);

		std::ifstream sourceIn(absoluteSourcePath, std::ios::in | std::ios::binary);
		if (!sourceIn.is_open()) return;

		std::ofstream out(outputPath, std::ios::out | std::ios::binary);
		if (!out) return;

		BinaryAssetHeader header;
		header.type = AssetType::NavMesh;
		header.uuid = data.uuid;
		header.version = ASSET_VERSION_NAVMESH;
		out.write(reinterpret_cast<const char*>(&header), sizeof(BinaryAssetHeader));

		out << sourceIn.rdbuf();

		sourceIn.close();
		out.close();
	}
}
