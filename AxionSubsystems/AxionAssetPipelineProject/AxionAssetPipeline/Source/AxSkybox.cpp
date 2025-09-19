#include "AxSkybox.h"

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/UUID.h"

#include <fstream>

namespace Axion::AAP {

	void SkyboxParser::createAxSkyFile(const SkyboxAssetData& data, const std::string& outputPath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Name" << YAML::Value << data.name;
		out << YAML::Key << "UUID" << YAML::Value << UUID().toString();
		out << YAML::Key << "Type" << YAML::Value << "Skybox";
		
		if (data.singleFileImport) {
			// -- Only one file with all cube faces --
			out << YAML::Key << "Texture" << YAML::Value << data.singleFilePath;
		}
		else {
			// -- Six files for each face --
			out << YAML::Key << "Textures" << YAML::Value;
			out << YAML::BeginMap;
			for (const auto& [key, path] : data.facesFilePaths) {
				out << YAML::Key << key << YAML::Value << path;
			}
			out << YAML::EndMap;
		}

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
	}

	void SkyboxParser::createAxSkyBinary(const SkyboxAssetData& data, const std::string& outputPath) {
		AX_CORE_ASSERT(false, "Creating a binary asset file is not supported yet");
	}

}
