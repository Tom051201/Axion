#pragma once

#include "AxionEngine/Vendor/miniaudio/miniaudio.h"

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/audio/AudioClip.h"

namespace Axion {

	struct AudioFileInfo {
		std::filesystem::path path;
		uint64_t fileSizeBytes = 0;
		float durationSeconds = 0.0f;
		uint32_t channels = 0;
		uint32_t sampleRate = 0;
	};



	class AudioManager {
	public:

		static void initialize();
		static void shutdown();

		static void setListener(const Vec3& position, const Vec3& forward, const Vec3& velocity = { 0,0,0 }, uint32_t listenerIndex = 0);

		static bool readAudioFileMetadata(const std::filesystem::path& path, AudioFileInfo& outInfo);
		static AudioClip::Mode decideMode(const AudioFileInfo& fileInfo);

		static ma_engine* getEngine();

	private:

		static bool s_inited;
		static ma_engine s_engine;

	};

}
