#pragma once

#include "AxionEngine/Source/audio/AudioClip.h"

#include <string>

namespace Axion::AAP {

	struct AudioAssetData {
		std::string name;
		std::string audioFilePath;
		std::string fileFormat;
		AudioClip::Mode mode;
	};

	class AudioParser {
	public:

		static void createAxAudioFile(const AudioAssetData& data, const std::string& outputPath);
		static void createAxAudioBinary(const AudioAssetData& data, const std::string& outputPath);

	};

}
