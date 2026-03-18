#include "AxAudio.h"

#include "AxionAssetPipeline/Source/core/BaseIncludes.h"

namespace Axion::AAP {

	void AudioParser::createTextFile(const AudioAssetData& data, const std::string& outputPath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Version" << YAML::Value << ASSET_VERSION_AUDIO;
		out << YAML::Key << "Name" << YAML::Value << data.name;
		out << YAML::Key << "UUID" << YAML::Value << data.uuid.toString();
		out << YAML::Key << "Type" << YAML::Value << "AudioClip";

		out << YAML::Key << "Format" << YAML::Value << data.fileFormat;
		out << YAML::Key << "Mode" << YAML::Value << EnumUtils::toString(data.mode);
		out << YAML::Key << "Source" << YAML::Value << data.audioFilePath;

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
		AX_CORE_LOG_TRACE("Created .axaudio file ({})", outputPath);
	}

	void AudioParser::createBinaryFile(const AudioAssetData& data, const std::string& outputPath) {
		AX_CORE_ASSERT(false, "Creating a binary asset file is not supported yet");
	}

}
