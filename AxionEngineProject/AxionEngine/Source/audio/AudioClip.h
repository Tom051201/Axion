#pragma once

#include "AxionEngine/Vendor/miniaudio/miniaudio.h"

#include "AxionEngine/Source/core/Math.h"

#include <filesystem>

namespace Axion {

	class AudioClip {
	public:

		enum class Mode { Memory, Stream };

		AudioClip(const std::filesystem::path& path, Mode mode);
		AudioClip(std::vector<uint8_t>&& audioData, Mode mode);
		~AudioClip();

		void release();

		ma_sound* getSound() { return &m_sound; }
		const ma_sound* getSound() const { return &m_sound; }
		Mode getMode() const { return m_mode; }
		const std::filesystem::path& getPath() const { return m_path; }

	private:

		std::filesystem::path m_path;
		Mode m_mode;
		ma_sound m_sound{};
		bool m_initialized = false;

		// -- Runtime Memory loading --
		std::vector<uint8_t> m_audioData;
		ma_decoder m_decoder{};
		bool m_fromMemory = false;

	};

}
