#pragma once

#include "AxionStudio/Source/core/Modal.h"
#include "AxionEngine/Source/audio/AudioClip.h"

#include <string>

namespace Axion {

	class AudioImportModal : public Modal {
	public:

		AudioImportModal(const char* name) : Modal(name) {}
		~AudioImportModal() override = default;

		void presetFromFile(const std::filesystem::path& sourceFile);

	private:

		void renderContent() override;
		void resetInputs() override;

		std::string m_name;
		std::string m_sourcePath;
		std::string m_outputPath;

		int m_importFormat = 0;
		const char* m_formatNames[3] = { "MP3", "WAV", "OGG" };

		int m_loadType = 0;
		AudioClip::Mode m_types[2] = { AudioClip::Mode::Stream, AudioClip::Mode::Memory };
		const char* m_typesNames[2] = { "Stream", "Memory" };

	};

}
