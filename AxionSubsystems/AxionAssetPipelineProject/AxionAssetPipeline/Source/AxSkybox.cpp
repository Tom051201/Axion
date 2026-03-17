#include "AxSkybox.h"

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/AssetManager.h"


#include <fstream>

namespace Axion::AAP {

	void SkyboxParser::createAxSkyFile(const SkyboxAssetData& data, const std::string& outputPath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

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

	void SkyboxParser::createAxSkyBinary(const SkyboxAssetData& data, const std::string& outputPath) {
		AX_CORE_ASSERT(false, "Creating a binary asset file is not supported yet");
	}

}
