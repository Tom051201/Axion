#pragma once

namespace Axion {

	class Timestep {
	public:
		
		Timestep(float time = 0.0f) : m_time(time) {}

		float getSeconds() const { return m_time; }
		float getMilliseconds() const { return m_time * 1000.0f; }

		// in seconds
		operator float() const { return m_time; }

	private:

		float m_time;

	};

}

