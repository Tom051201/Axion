#include "AxSkybox.h"

#include "AxionAssetPipeline/Source/core/BaseIncludes.h"

namespace Axion::AAP {

	void SkyboxParser::createTextFile(const SkyboxAssetData& data, const std::string& outputPath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Version" << YAML::Value << ASSET_VERSION_SKYBOX;
		out << YAML::Key << "Name" << YAML::Value << data.name;
		out << YAML::Key << "UUID" << YAML::Value << data.uuid.toString();
		out << YAML::Key << "Type" << YAML::Value << "Skybox";
		
		UUID textureUUID = AssetManager::getAssetUUID(AssetManager::getAbsolute(data.textureCubePath));
		out << YAML::Key << "TextureCube" << YAML::Value << textureUUID.toString();

		if (!data.pipelinePath.empty()) {
			UUID pipelineUUID = AssetManager::getAssetUUID(AssetManager::getAbsolute(data.pipelinePath));
			out << YAML::Key << "Pipeline" << YAML::Value << pipelineUUID.toString();
		}

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
		AX_CORE_LOG_TRACE("Created .axsky file ({})", outputPath);
	}

	void SkyboxParser::createBinaryFile(const SkyboxAssetData& data, const std::string& outputPath) {
		std::ofstream out(outputPath, std::ios::out | std::ios::binary);

		SkyboxBinaryHeader header = {};
		header.assetHeader.type = AssetType::Skybox;
		header.assetHeader.uuid = data.uuid;
		header.assetHeader.version = ASSET_VERSION_SKYBOX;
		header.textureCubeUUID = AssetManager::getAssetUUID(AssetManager::getAbsolute(data.textureCubePath));
		if (!data.pipelinePath.empty()) {
			header.pipelineUUID = AssetManager::getAssetUUID(AssetManager::getAbsolute(data.pipelinePath));
		}
		else {
			header.pipelineUUID = UUID(0, 0);
		}
		out.write(reinterpret_cast<const char*>(&header), sizeof(SkyboxBinaryHeader));

		out.close();
		AX_CORE_LOG_TRACE("Baked binary Skybox to {}", outputPath);
	}

}
