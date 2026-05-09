#include "axpch.h"
#include "AudioSource.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/BinaryHeaders.h"
#include "AxionEngine/Source/audio/AudioClip.h"
#include "AxionEngine/Source/audio/AudioManager.h"

namespace Axion {

	AudioSource::AudioSource(AudioSource&& other) noexcept {
		*this = std::move(other);
	}

	AudioSource& AudioSource::operator=(AudioSource&& other) noexcept {
		if (this != &other) {
			release();

			m_clipHandle = other.m_clipHandle;
			m_sound = other.m_sound;
			m_streamDecoder = other.m_streamDecoder;
			m_streamContext = other.m_streamContext;
			m_initialized = other.m_initialized;
			m_ownsDecoder = other.m_ownsDecoder;
			m_onEndCallback = std::move(other.m_onEndCallback);
			m_paused = other.m_paused;
			m_volume = other.m_volume;
			m_pitch = other.m_pitch;
			m_pan = other.m_pan;
			m_position = other.m_position;
			m_velocity = other.m_velocity;
			m_minDistance = other.m_minDistance;
			m_maxDistance = other.m_maxDistance;
			m_dopplerFactor = other.m_dopplerFactor;

			other.m_sound = nullptr;
			other.m_streamDecoder = nullptr;
			other.m_streamContext = nullptr;
			other.m_initialized = false;
		}
		return *this;
	}

	AudioSource::AudioSource(AssetHandle<AudioClip> handle) {
		m_clipHandle = handle;
		Ref<AudioClip> clip = AssetManager::get<AudioClip>(handle);
		if (!clip) return;

		m_sound = new ma_sound();

		// -- Memory Sounds --
		if (clip->getMode() == AudioClip::Mode::Memory) {
			if (ma_sound_init_copy(AudioManager::getEngine(), clip->getSound(), 0, nullptr, m_sound) != MA_SUCCESS) {
				AX_CORE_LOG_ERROR("Failed to copy sound for source");
				delete m_sound; m_sound = nullptr;
				return;
			}
		}
		// -- Streaming Sounds --
		else {
			if (clip->getPath().extension() == ".axbin") {

				m_streamContext = new AudioStreamContext();
				m_streamContext->stream.open(clip->getPath(), std::ios::in | std::ios::binary);

				if (!m_streamContext->stream.is_open()) {
					AX_CORE_LOG_ERROR("Failed to open binary audio file: {}", clip->getPath().string());
					delete m_sound; m_sound = nullptr;
					delete m_streamContext; m_streamContext = nullptr;
					return;
				}

				BinaryAssetHeader header;
				uint32_t mode;
				uint64_t dataSize;
				m_streamContext->stream.read(reinterpret_cast<char*>(&header), sizeof(BinaryAssetHeader));
				m_streamContext->stream.read(reinterpret_cast<char*>(&mode), sizeof(uint32_t));
				m_streamContext->stream.read(reinterpret_cast<char*>(&dataSize), sizeof(uint64_t));

				m_streamContext->dataOffset = m_streamContext->stream.tellg();
				m_streamContext->dataSize = dataSize;

				m_streamDecoder = new ma_decoder();
				ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);

				if (ma_decoder_init(miniaudioRead, miniaudioSeek, m_streamContext, &config, m_streamDecoder) != MA_SUCCESS) {
					AX_CORE_LOG_ERROR("Failed to init binary stream decoder");
					return;
				}

				if (ma_sound_init_from_data_source(AudioManager::getEngine(), m_streamDecoder, MA_SOUND_FLAG_STREAM, nullptr, m_sound) != MA_SUCCESS) {
					AX_CORE_LOG_ERROR("Failed to init streaming sound");
					return;
				}
				m_ownsDecoder = true;
			}
			else {
				if (ma_sound_init_from_file(AudioManager::getEngine(), clip->getPath().string().c_str(), MA_SOUND_FLAG_STREAM, nullptr, nullptr, m_sound) != MA_SUCCESS) {
					AX_CORE_LOG_ERROR("Failed to init streaming sound from file");
					delete m_sound; m_sound = nullptr;
					return;
				}
				m_ownsDecoder = false;
			}
		}
		m_initialized = true;
	}

	AudioSource::~AudioSource() {
		release();
	}

	void AudioSource::release() {
		if (m_initialized && m_sound != nullptr) {
			ma_sound_stop(m_sound);
			ma_sound_uninit(m_sound);
			delete m_sound;
			m_sound = nullptr;

			if (m_ownsDecoder && m_streamDecoder != nullptr) {
				ma_decoder_uninit(m_streamDecoder);
				delete m_streamDecoder;
				m_streamDecoder = nullptr;

				if (m_streamContext) {
					if (m_streamContext->stream.is_open()) {
						m_streamContext->stream.close();
					}
					delete m_streamContext;
					m_streamContext = nullptr;
				}
			}
		}
		m_initialized = false;
	}

	void AudioSource::play() {
		if (!m_sound) return;
		m_paused = false;
		ma_sound_start(m_sound);
	}

	void AudioSource::stop() {
		if (!m_sound) return;
		ma_sound_stop(m_sound);
		ma_sound_seek_to_pcm_frame(m_sound, 0);
		m_paused = false;
	}

	void AudioSource::pause() {
		if (!m_sound) return;
		ma_sound_stop(m_sound);
		m_paused = true;
	}

	void AudioSource::resume() {
		if (!m_sound) return;
		if (m_paused) {
			ma_sound_start(m_sound);
			m_paused = false;
		}
	}

	void AudioSource::loop(bool loop) {
		if (!m_sound) return;
		ma_sound_set_looping(m_sound, loop ? MA_TRUE : MA_FALSE);
	}

	void AudioSource::setOnEndCallback(std::function<void()> callback) {
		if (!m_sound) return;
		ma_sound_set_end_callback(m_sound, [](void* pUserData, ma_sound* pSound) {
			auto* callback = reinterpret_cast<std::function<void()>*>(pUserData);
			if (callback) (*callback)();
		}, &m_onEndCallback);
	}

	bool AudioSource::isPlaying() const {
		if (!m_sound) return false;
		return ma_sound_is_playing(m_sound);
	}

	bool AudioSource::isLooping() const {
		if (!m_sound) return false;
		return ma_sound_is_looping(m_sound);
	}

	bool AudioSource::isStreaming() const {
		if (!m_sound) return false;
		Ref<AudioClip> clip = AssetManager::get<AudioClip>(m_clipHandle);
		return clip->getMode() == AudioClip::Mode::Stream;
	}

	bool AudioSource::isSpatial() const {
		if (!m_sound) return false;
		return ma_sound_is_spatialization_enabled(m_sound);
	}

	bool AudioSource::isPaused() const {
		return m_paused;
	}

	void AudioSource::setVolume(float vol) {
		if (!m_sound) return;
		m_volume = vol;
		ma_sound_set_volume(m_sound, vol);
	}

	void AudioSource::setPitch(float pitch) {
		if (!m_sound) return;
		m_pitch = pitch;
		ma_sound_set_pitch(m_sound, pitch);
	}

	void AudioSource::setPan(float pan) {
		if (!m_sound) return;
		m_pan = pan;
		ma_sound_set_pan(m_sound, pan);
	}

	void AudioSource::enableSpatial() {
		if (!m_sound) return;
		ma_sound_set_spatialization_enabled(m_sound, MA_TRUE);
	}

	void AudioSource::disableSpatial() {
		if (!m_sound) return;
		ma_sound_set_spatialization_enabled(m_sound, MA_FALSE);
	}

	void AudioSource::setPosition(const Vec3& pos) {
		if (!m_sound) return;
		m_position = pos;
		ma_sound_set_position(m_sound, pos.x, pos.y, pos.z);
	}

	void AudioSource::setVelocity(const Vec3& vel) {
		if (!m_sound) return;
		m_velocity = vel;
		ma_sound_set_velocity(m_sound, vel.x, vel.y, vel.z);
	}

	void AudioSource::setMinDistance(float minDis) {
		if (!m_sound) return;
		m_minDistance = minDis;
		ma_sound_set_min_distance(m_sound, minDis);
	}

	void AudioSource::setMaxDistance(float maxDis) {
		if (!m_sound) return;
		m_maxDistance = maxDis;
		ma_sound_set_max_distance(m_sound, maxDis);
	}

	void AudioSource::setDopplerFactor(float factor) {
		if (!m_sound) return;
		m_dopplerFactor = factor;
		ma_sound_set_doppler_factor(m_sound, factor);
	}

	ma_result AudioSource::miniaudioRead(ma_decoder* decoder, void* bufferOut, size_t bytesToRead, size_t* bytesRead) {
		AudioStreamContext* ctx = static_cast<AudioStreamContext*>(decoder->pUserData);
		if (!ctx || !ctx->stream.is_open()) return MA_ERROR;

		ctx->stream.read(static_cast<char*>(bufferOut), bytesToRead);
		size_t readBytes = static_cast<size_t>(ctx->stream.gcount());

		if (bytesRead != nullptr) {
			*bytesRead = readBytes;
		}

		if (readBytes == 0 && bytesToRead > 0) {
			return MA_AT_END;
		}

		return MA_SUCCESS;
	}

	ma_result AudioSource::miniaudioSeek(ma_decoder* decoder, ma_int64 byteOffset, ma_seek_origin origin) {
		AudioStreamContext* ctx = static_cast<AudioStreamContext*>(decoder->pUserData);
		if (!ctx || !ctx->stream.is_open()) return MA_ERROR;

		if (origin == ma_seek_origin_start) {
			ctx->stream.seekg(ctx->dataOffset + byteOffset, std::ios::beg);
		}
		else if (origin == ma_seek_origin_current) {
			ctx->stream.seekg(byteOffset, std::ios::cur);
		}
		else if (origin == ma_seek_origin_end) {
			ctx->stream.seekg((ctx->dataOffset + ctx->dataSize) + byteOffset, std::ios::beg);
		}

		return ctx->stream.fail() ? MA_ERROR : MA_SUCCESS;
	}

}
