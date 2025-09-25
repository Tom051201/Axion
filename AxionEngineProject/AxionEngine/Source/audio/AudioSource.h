#pragma once

#include "AxionEngine/Vendor/miniaudio/miniaudio.h"

#include "AxionEngine/Source/core/AssetHandle.h"
#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/audio/AudioClip.h"

namespace Axion {

	class AudioSource {
	public:

		AudioSource(AssetHandle<AudioClip> handle);
		~AudioSource();

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

	public:

		AssetHandle<AudioClip> m_clipHandle;
		ma_sound m_memorySound{};
		ma_sound m_streamSound{};
		ma_sound* m_instance{};
		bool m_initialized = false;

		std::function<void()> m_onEndCallback;
		ma_uint64 m_pausePosition = 0;
		bool m_paused = false;
		float m_volume = 1.0f;
		float m_pitch = 1.0f;
		float m_pan = 0.0f;

		Vec3 m_position;
		Vec3 m_velocity;
		float m_minDistance = 1.0f;
		float m_maxDistance = 100.0f;
		float m_dopplerFactor = 1.0f;

	};

}
