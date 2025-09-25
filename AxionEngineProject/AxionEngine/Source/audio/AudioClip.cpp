#include "axpch.h"
#include "AudioClip.h"

#include "AxionEngine/Source/audio/AudioManager.h"

namespace Axion {

	AudioClip::AudioClip(const std::string& path, Mode mode)
		: m_path(path), m_mode(mode) {

		ma_uint32 flags = (mode == Mode::Memory) ? MA_SOUND_FLAG_DECODE : MA_SOUND_FLAG_STREAM;

		if (ma_sound_init_from_file(AudioManager::getEngine(), path.c_str(), flags, nullptr, nullptr, &m_sound) != MA_SUCCESS) {
			AX_CORE_LOG_ERROR("Failed to load audio: {}", path);
		}
		m_initialized = true;
	}

	AudioClip::~AudioClip() {
		release();
	}

	void AudioClip::release() {
		if (m_initialized) { ma_sound_uninit(&m_sound); }
		m_initialized = false;
	}

}
