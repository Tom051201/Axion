#pragma once

#include <string>
#include <filesystem>

#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/audio/AudioClip.h"

#include "AxionAssetPipeline/Source/core/AssetFormats.h"

namespace Axion::AAP {

	struct AudioAssetData {
		UUID uuid;
		std::string name;
		std::filesystem::path audioFilePath;
		AudioFormat fileFormat = AudioFormat::None;
		AudioClip::Mode mode;
	};

	class AudioParser {
	public:

		static void createTextFile(const AudioAssetData& data, const std::filesystem::path& outputPath);
		static void createBinaryFile(const AudioAssetData& data, const std::filesystem::path& outputPath);

	};

}
