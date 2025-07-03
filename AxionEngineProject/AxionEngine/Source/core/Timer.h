#pragma once

#include <chrono>

#include "AxionEngine/Source/core/Logging.h"

namespace Axion {

	template <typename Fn>
	class ScopedTimer {
	public:

		ScopedTimer(const char* name, Fn&& fn) : m_name(name), m_func(fn), m_stopped(false) {
			m_start = std::chrono::high_resolution_clock::now();
		}

		~ScopedTimer() {
			if (!m_stopped) stop();
		}

		void stop() {
			auto endTime = std::chrono::high_resolution_clock::now();

			int64_t start = std::chrono::time_point_cast<std::chrono::microseconds>(m_start).time_since_epoch().count();
			int64_t end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();

			m_stopped = true;

			float duration = (end - start) * 0.001f;
			m_func({m_name, duration});
		}

	private:

		const char* m_name;
		Fn m_func;
		std::chrono::time_point<std::chrono::steady_clock> m_start;
		bool m_stopped;
	};

}
