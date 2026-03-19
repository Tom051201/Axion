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

	AudioClip::AudioClip(std::vector<uint8_t>&& audioData, Mode mode)
		: m_audioData(std::move(audioData)), m_mode(mode), m_fromMemory(true) {

		ma_uint32 flags = (mode == Mode::Memory) ? MA_SOUND_FLAG_DECODE : MA_SOUND_FLAG_STREAM;

		if (ma_decoder_init_memory(m_audioData.data(), m_audioData.size(), nullptr, &m_decoder) != MA_SUCCESS) {
			AX_CORE_LOG_ERROR("Failed to init miniaudio decoder from memory!");
			return;
		}

		if (ma_sound_init_from_data_source(AudioManager::getEngine(), &m_decoder, flags, nullptr, &m_sound) != MA_SUCCESS) {
			AX_CORE_LOG_ERROR("Failed to init miniaudio sound from data source!");
			return;
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
