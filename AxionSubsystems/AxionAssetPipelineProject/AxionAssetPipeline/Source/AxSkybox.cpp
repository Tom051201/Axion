#include "AxSkybox.h"

#include "AxionAssetPipeline/Source/core/BaseIncludes.h"

#include "AxionEngine/Source/core/AssetManager.h"

namespace Axion::AAP {

	void SkyboxParser::createTextFile(const SkyboxAssetData& data, const std::string& outputPath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Version" << YAML::Value << ASSET_VERSION_SKYBOX;
		out << YAML::Key << "Name" << YAML::Value << data.name;
		out << YAML::Key << "UUID" << YAML::Value << data.uuid.toString();
		out << YAML::Key << "Type" << YAML::Value << "Skybox";
		
		if (data.singleFileImport) {
			// -- Only one file with all cube faces --
			std::string texturePath = data.singleFilePath;
			out << YAML::Key << "Texture" << YAML::Value << texturePath;
		}
		else {
			// -- Six files for each face --
			out << YAML::Key << "Textures" << YAML::Value;
			out << YAML::BeginMap;
			for (const auto& [key, path] : data.facesFilePaths) { // TODO REWORK
				UUID textureUUID = AssetManager::getAssetUUID(AssetManager::getAbsolute(path));
				out << YAML::Key << key << YAML::Value << textureUUID.toString();
			}
			out << YAML::EndMap;
		}

		UUID pipelineUUID = AssetManager::getAssetUUID(AssetManager::getAbsolute(data.pipelinePath));
		out << YAML::Key << "Pipeline" << YAML::Value << pipelineUUID.toString();

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
		AX_CORE_LOG_TRACE("Created .axsky file ({})", outputPath);
	}

	void SkyboxParser::createBinaryFile(const SkyboxAssetData& data, const std::string& outputPath) {
		AX_CORE_ASSERT(false, "Creating a binary asset file is not supported yet");
	}

}
