#include "axpch.h"
#include "AudioManager.h"

#define MINIAUDIO_IMPLEMENTATION
#include "AxionEngine/Vendor/miniaudio/miniaudio.h"

namespace Axion {

	ma_engine AudioManager::s_engine;
	bool AudioManager::s_inited = false;

	void AudioManager::initialize() {
		if (s_inited) {
			AX_CORE_LOG_WARN("AudioManager already initialized");
			return;
		}

		ma_result result = ma_engine_init(nullptr, &s_engine);
		if (result != MA_SUCCESS) {
			AX_CORE_LOG_ERROR("Failed to initialize audio engine");
			return;
		}

		s_inited = true;
		AX_CORE_LOG_INFO("AudioManager initialized successfully");
	}

	void AudioManager::shutdown() {
		if (!s_inited) return;

		ma_engine_stop(&s_engine);
		ma_engine_uninit(&s_engine);
		s_inited = false;

		AX_CORE_LOG_INFO("AudioManager shutdown");
	}

	void AudioManager::setListener(const Vec3& position, const Vec3& forward, const Vec3& velocity, uint32_t listenerIndex) {
		if (!s_inited) return;

		ma_engine_listener_set_position(&s_engine, listenerIndex, position.x, position.y, position.z);
		ma_engine_listener_set_velocity(&s_engine, listenerIndex, velocity.x, velocity.y, velocity.z);
		ma_engine_listener_set_direction(&s_engine, listenerIndex, forward.x, forward.y, forward.z);
	}

	ma_engine* AudioManager::getEngine() {
		return &s_engine;
	}

}
