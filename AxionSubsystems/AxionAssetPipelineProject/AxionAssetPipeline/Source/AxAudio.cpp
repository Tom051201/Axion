#include "axpch.h"
#include "AxAudio.h"

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/core/EnumUtils.h"

#include <fstream>

namespace Axion::AAP {

	void AudioParser::createAxAudioFile(const AudioAssetData& data, const std::string& outputPath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Name" << YAML::Value << data.name;
		out << YAML::Key << "UUID" << YAML::Value << UUID::generate().toString();
		out << YAML::Key << "Type" << YAML::Value << "AudioClip";

		out << YAML::Key << "Format" << YAML::Value << data.fileFormat;
		out << YAML::Key << "Mode" << YAML::Value << EnumUtils::toString(data.mode);
		out << YAML::Key << "Source" << YAML::Value << data.audioFilePath;

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
		AX_CORE_LOG_TRACE("Created .axaudio file ({})", outputPath);
	}

	void AudioParser::createAxAudioBinary(const AudioAssetData& data, const std::string& outputPath) {
		AX_CORE_ASSERT(false, "Creating a binary asset file is not supported yet");
	}

}
