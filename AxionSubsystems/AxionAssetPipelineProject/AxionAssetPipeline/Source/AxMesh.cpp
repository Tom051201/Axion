#include "AxMesh.h"

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/UUID.h"

#include <fstream>

namespace Axion::AAP {

	void MeshParser::createAxMeshFile(const MeshAssetData& data, const std::string& outputPath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Name" << YAML::Value << data.name;
		out << YAML::Key << "UUID" << YAML::Value << UUID::generate().toString();
		out << YAML::Key << "Type" << YAML::Value << "Mesh";
		out << YAML::Key << "Format" << YAML::Value << data.fileFormat;
		out << YAML::Key << "Source" << YAML::Value << data.filePath;

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
	}

	void MeshParser::createAxMeshBinary(const MeshAssetData& data, const std::string& outputPath) {
		AX_CORE_ASSERT(false, "Creating a binary asset file is not supported yet");
	}

}
