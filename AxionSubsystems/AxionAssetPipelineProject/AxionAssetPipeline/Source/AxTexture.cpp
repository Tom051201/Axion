#include "AxTexture.h"

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/UUID.h"

#include <fstream>

namespace Axion::AAP {

	void Texture2DParser::createAxTexFile(const Texture2DAssetData& data, const std::string& outputPath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Name" << YAML::Value << data.name;
		out << YAML::Key << "UUID" << YAML::Value << UUID::generate().toString();
		out << YAML::Key << "Type" << YAML::Value << "Texture2D";
		out << YAML::Key << "Format" << YAML::Value << data.fileFormat;
		out << YAML::Key << "Source" << YAML::Value << data.filePath;

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
		AX_CORE_LOG_TRACE("Created .axtex file ({})", outputPath);
	}

	void Texture2DParser::createAxTexBinary(const Texture2DAssetData& data, const std::string& outputPath) {
		AX_CORE_ASSERT(false, "Creating a binary asset file is not supported yet");
	}

}
