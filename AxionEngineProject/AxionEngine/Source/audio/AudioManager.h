#pragma once

#include "AxionEngine/Vendor/miniaudio/miniaudio.h"

#include "AxionEngine/Source/core/Math.h"

namespace Axion {

	class AudioManager {
	public:

		static void initialize();
		static void shutdown();

		static void setListener(const Vec3& position, const Vec3& forward, const Vec3& velocity = { 0,0,0 }, uint32_t listenerIndex = 0);

		static ma_engine* getEngine();

	private:

		static bool s_inited;
		static ma_engine s_engine;

	};

}
