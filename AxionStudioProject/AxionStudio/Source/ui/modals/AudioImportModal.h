#pragma once

#include <string>
#include <filesystem>
#include <functional>
#include <memory>

#include <Silica/include/SWidget.h>

#include "AxionEngine/Source/audio/AudioClip.h"

namespace Silica {
	class SBox;
}

namespace Axion {

	class AudioImportModal {
	public:

		AudioImportModal() = default;
		~AudioImportModal() = default;

		void presetFromFile(const std::filesystem::path& sourceFile);

		Silica::WidgetPtr getWidget(std::function<void()> onClose);

	private:

		void rebuildUI();
		void rebuildUI_Internal();
		void resetInputs();

		std::string m_name;
		std::string m_sourcePath;
		std::string m_outputPath;

		int m_importFormat = 0;
		const char* m_formatNames[3] = { "MP3", "WAV", "OGG" };

		int m_loadType = 0;
		AudioClip::Mode m_types[2] = { AudioClip::Mode::Stream, AudioClip::Mode::Memory };
		const char* m_typesNames[2] = { "Stream", "Memory" };

		// -- Silica --
		std::shared_ptr<Silica::SBox> m_uiRoot;
		std::function<void()> m_onClose;
		bool m_rebuildQueued = false;

	};

}
