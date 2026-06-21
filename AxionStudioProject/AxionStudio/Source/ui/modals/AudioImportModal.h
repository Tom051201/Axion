#pragma once

#include "AxionEngine/Source/audio/AudioClip.h"
#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/FontAtlas.h"

#include <string>
#include <filesystem>
#include <functional>

namespace Axion {

	class AudioImportModal {
	public:

		AudioImportModal() = default;
		~AudioImportModal() = default;

		void presetFromFile(const std::filesystem::path& sourceFile);

		Silica::WidgetPtr getWidget(Silica::FontAtlas* font, std::function<void()> onClose);

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
		Silica::FontAtlas* m_font = nullptr;
		std::function<void()> m_onClose;
		bool m_rebuildQueued = false;

	};

}
