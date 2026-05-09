#pragma once

#include "AxionEngine/Vendor/miniaudio/miniaudio.h"

#include "AxionEngine/Source/core/AssetHandle.h"
#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/audio/AudioClip.h"

namespace Axion {

	struct AudioStreamContext {
		std::ifstream stream;
		uint64_t dataOffset = 0;
		uint64_t dataSize = 0;
	};

	class AudioSource {
	public:

		AudioSource(AssetHandle<AudioClip> handle);
		AudioSource(AudioSource&& other) noexcept;
		AudioSource(const AudioSource&) = delete;
		AudioSource& operator=(const AudioSource&) = delete;
		~AudioSource();

		AudioSource& operator=(AudioSource&& other) noexcept;

		void release();

		void play();
		void stop();
		void pause();
		void resume();
		void loop(bool loop);
		void setOnEndCallback(std::function<void()> callback);

		bool isPlaying() const;
		bool isLooping() const;
		bool isStreaming() const;
		bool isSpatial() const;
		bool isPaused() const;

		void setVolume(float vol);
		void setPitch(float pitch);
		void setPan(float pan);
		float getVolume() const { return m_volume; }
		float getPitch() const { return m_pitch; }
		float getPan() const { return m_pan; }

		void enableSpatial();
		void disableSpatial();
		void setPosition(const Vec3& pos);
		void setVelocity(const Vec3& vel);
		void setMinDistance(float minDis);
		void setMaxDistance(float maxDis);
		void setDopplerFactor(float factor);
		const Vec3& getPosition() const { return m_position; }
		const Vec3& getVelocity() const { return m_velocity; }
		float getMinDistance() const { return m_minDistance; }
		float getMaxDistance() const { return m_maxDistance; }
		float getDopplerFactor() const { return m_dopplerFactor; }
		Vec3& getPosition() { return m_position; }
		Vec3& getVelocity() { return m_velocity; }

		AssetHandle<AudioClip> getClipHandle() const { return m_clipHandle; }

	private:

		AssetHandle<AudioClip> m_clipHandle;

		ma_sound* m_sound = nullptr;
		ma_decoder* m_streamDecoder = nullptr;
		AudioStreamContext* m_streamContext = nullptr;

		bool m_initialized = false;
		bool m_ownsDecoder = false;
		std::function<void()> m_onEndCallback;

		bool m_paused = false;
		float m_volume = 1.0f;
		float m_pitch = 1.0f;
		float m_pan = 0.0f;

		std::ifstream m_binaryStream;
		uint64_t m_binaryDataOffset = 0;
		uint64_t m_binaryDataSize = 0;

		Vec3 m_position;
		Vec3 m_velocity;
		float m_minDistance = 1.0f;
		float m_maxDistance = 100.0f;
		float m_dopplerFactor = 1.0f;

		static ma_result miniaudioRead(ma_decoder* decoder, void* bufferOut, size_t bytesToRead, size_t* bytesRead);
		static ma_result miniaudioSeek(ma_decoder* decoder, ma_int64 byteOffset, ma_seek_origin origin);

	};

}
