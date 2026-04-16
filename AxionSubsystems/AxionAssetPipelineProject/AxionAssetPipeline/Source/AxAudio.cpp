#include "AxAudio.h"

#include "AxionAssetPipeline/Source/core/BaseIncludes.h"

namespace Axion::AAP {

	void AudioParser::createTextFile(const AudioAssetData& data, const std::filesystem::path& outputPath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Version" << YAML::Value << ASSET_VERSION_AUDIO;
		out << YAML::Key << "Name" << YAML::Value << data.name;
		out << YAML::Key << "UUID" << YAML::Value << data.uuid.toString();
		out << YAML::Key << "Type" << YAML::Value << "AudioClip";

		out << YAML::Key << "Format" << YAML::Value << FormatUtils::audioFormatToString(data.fileFormat);
		out << YAML::Key << "Mode" << YAML::Value << EnumUtils::toString(data.mode);
		out << YAML::Key << "Source" << YAML::Value << data.audioFilePath.generic_string();

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
		AX_CORE_LOG_TRACE("Created .axaudio file ({})", outputPath.string());
	}

	void AudioParser::createBinaryFile(const AudioAssetData& data, const std::filesystem::path& outputPath) {
		std::ifstream audioFile(data.audioFilePath, std::ios::in | std::ios::binary);
		if (!audioFile) {
			AX_CORE_LOG_ERROR("Failed to open Source Audio File: {}", data.audioFilePath.string());
			return;
		}

		audioFile.seekg(0, std::ios::end);
		size_t audioFileSize = audioFile.tellg();
		audioFile.seekg(0, std::ios::beg);

		std::vector<uint8_t> audioData(audioFileSize);
		audioFile.read(reinterpret_cast<char*>(audioData.data()), audioFileSize);
		audioFile.close();

		std::ofstream out(outputPath, std::ios::out | std::ios::binary);
		if (!out) {
			AX_CORE_LOG_ERROR("Failed to create binary file: {}", outputPath.string());
			return;
		}

		// -- Write Header --
		BinaryAssetHeader header;
		header.type = AssetType::AudioClip;
		header.uuid = data.uuid;
		header.version = ASSET_VERSION_AUDIO;
		out.write(reinterpret_cast<const char*>(&header), sizeof(BinaryAssetHeader));

		// -- Write Metadata --
		uint32_t mode = static_cast<uint32_t>(data.mode);
		out.write(reinterpret_cast<const char*>(&mode), sizeof(uint32_t));

		// -- Write Audio Data Size --
		uint64_t dataSize = static_cast<uint64_t>(audioFileSize);
		out.write(reinterpret_cast<const char*>(&dataSize), sizeof(uint64_t));

		// -- Write Raw File Bytes --
		out.write(reinterpret_cast<const char*>(audioData.data()), audioFileSize);

		out.close();
		AX_CORE_LOG_TRACE("Baked binary audio to {}", outputPath.string());
	}

}
