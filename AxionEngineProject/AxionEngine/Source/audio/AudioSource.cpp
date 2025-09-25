#include "axpch.h"
#include "AudioSource.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/audio/AudioClip.h"
#include "AxionEngine/Source/audio/AudioManager.h"

namespace Axion {

	AudioSource::AudioSource(AssetHandle<AudioClip> handle) {
		m_clipHandle = handle;
		Ref<AudioClip> clip = AssetManager::get<AudioClip>(handle);

		// -- Memory sounds --
		if (clip->getMode() == AudioClip::Mode::Memory) {
			if (ma_sound_init_copy(AudioManager::getEngine(), clip->getSound(), 0, nullptr, &m_memorySound) != MA_SUCCESS) {
				AX_CORE_LOG_ERROR("Failed to copy sound for source");
				m_instance = nullptr;
				m_initialized = false;
				return;
			}
			m_instance = &m_memorySound;
		}
		// -- Streaming sounds --
		else {
			if (ma_sound_init_from_file(AudioManager::getEngine(), clip->getPath().c_str(), MA_SOUND_FLAG_STREAM, nullptr, nullptr, &m_streamSound) != MA_SUCCESS) {
				AX_CORE_LOG_ERROR("Failed to init streaming sound for source");
				m_instance = nullptr;
				m_initialized = false;
				return;
			}
			m_instance = &m_streamSound;
		}
		m_initialized = true;
	}

	AudioSource::~AudioSource() {
		release();
	}

	void AudioSource::release() {
		if (m_initialized) {
			Ref<AudioClip> clip = AssetManager::get<AudioClip>(m_clipHandle);
			if (clip->getMode() == AudioClip::Mode::Memory) {
				ma_sound_uninit(&m_memorySound);
			}
			else {
				ma_sound_uninit(&m_streamSound);
			}
			m_instance = nullptr;
		}
		m_initialized = false;
	}

	void AudioSource::play() {
		if (!m_instance) return;
		ma_sound_start(m_instance);
	}

	void AudioSource::stop() {
		if (!m_instance) return;
		ma_sound_stop(m_instance);
	}

	void AudioSource::pause() {
		if (!m_instance) return;
		ma_sound_get_cursor_in_pcm_frames(m_instance, &m_pausePosition);
		ma_sound_stop(m_instance);
		m_paused = true;
	}

	void AudioSource::resume() {
		if (!m_instance) return;
		if (m_paused) {
			ma_sound_seek_to_pcm_frame(m_instance, m_pausePosition);
			ma_sound_start(m_instance);
		}
	}

	void AudioSource::loop(bool loop) {
		if (!m_instance) return;
		ma_sound_set_looping(m_instance, loop ? MA_TRUE : MA_FALSE);
	}

	void AudioSource::setOnEndCallback(std::function<void()> callback) {
		if (!m_instance) return;
		ma_sound_set_end_callback(m_instance, [](void* pUserData, ma_sound* pSound) {
			auto* callback = reinterpret_cast<std::function<void()>*>(pUserData);
			if (callback) (*callback)();
		}, &m_onEndCallback);
	}

	bool AudioSource::isPlaying() const {
		if (!m_instance) return false;
		return ma_sound_is_playing(m_instance);
	}

	bool AudioSource::isLooping() const {
		if (!m_instance) return false;
		return ma_sound_is_looping(m_instance);
	}

	bool AudioSource::isStreaming() const {
		if (!m_instance) return false;
		Ref<AudioClip> clip = AssetManager::get<AudioClip>(m_clipHandle);
		return clip->getMode() == AudioClip::Mode::Stream;
	}

	bool AudioSource::isSpatial() const {
		if (!m_instance) return false;
		return ma_sound_is_spatialization_enabled(m_instance);
	}

	bool AudioSource::isPaused() const {
		return m_paused;
	}

	void AudioSource::setVolume(float vol) {
		if (!m_instance) return;
		m_volume = vol;
		ma_sound_set_volume(m_instance, vol);
	}

	void AudioSource::setPitch(float pitch) {
		if (!m_instance) return;
		m_pitch = pitch;
		ma_sound_set_pitch(m_instance, pitch);
	}

	void AudioSource::setPan(float pan) {
		if (!m_instance) return;
		m_pan = pan;
		ma_sound_set_pan(m_instance, pan);
	}

	void AudioSource::enableSpatial() {
		if (!m_instance) return;
		ma_sound_set_spatialization_enabled(m_instance, MA_TRUE);
	}

	void AudioSource::disableSpatial() {
		if (!m_instance) return;
		ma_sound_set_spatialization_enabled(m_instance, MA_FALSE);
	}

	void AudioSource::setPosition(const Vec3& pos) {
		if (!m_instance) return;
		m_position = pos;
		ma_sound_set_position(m_instance, pos.x, pos.y, pos.z);
	}

	void AudioSource::setVelocity(const Vec3& vel) {
		if (!m_instance) return;
		m_velocity = vel;
		ma_sound_set_velocity(m_instance, vel.x, vel.y, vel.z);
	}

	void AudioSource::setMinDistance(float minDis) {
		if (!m_instance) return;
		m_minDistance = minDis;
		ma_sound_set_min_distance(m_instance, minDis);
	}

	void AudioSource::setMaxDistance(float maxDis) {
		if (!m_instance) return;
		m_maxDistance = maxDis;
		ma_sound_set_max_distance(m_instance, maxDis);
	}

	void AudioSource::setDopplerFactor(float factor) {
		if (!m_instance) return;
		m_dopplerFactor = factor;
		ma_sound_set_doppler_factor(m_instance, factor);
	}

}
