#pragma once

#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/audio/AudioClip.h"

#include <string>

namespace Axion::AAP {

	struct AudioAssetData {
		UUID uuid;
		std::string name;
		std::string audioFilePath;
		std::string fileFormat;
		AudioClip::Mode mode;
	};

	class AudioParser {
	public:

		static void createTextFile(const AudioAssetData& data, const std::string& outputPath);
		static void createBinaryFile(const AudioAssetData& data, const std::string& outputPath);

	};

}
