#pragma once

#include "AxionEngine/Vendor/miniaudio/miniaudio.h"

#include "AxionEngine/Source/core/Math.h"

#include <string>

namespace Axion {

	class AudioClip {
	public:

		enum class Mode { Memory, Stream };

		AudioClip(const std::string& path, Mode mode);
		~AudioClip();

		void release();

		ma_sound* getSound() { return &m_sound; }
		const ma_sound* getSound() const { return &m_sound; }
		Mode getMode() const { return m_mode; }
		const std::string& getPath() const { return m_path; }

	private:

		std::string m_path;
		Mode m_mode;
		ma_sound m_sound{};
		bool m_initialized = false;

	};

}
