#pragma once

#include <filesystem>

#include "AxionEngine/Source/audio/AudioClip.h"

#include "AxionStudio/Source/ui/ModalBase.h"

namespace Axion {

	class AudioImportModal : public ModalBase {
	public:

		AudioImportModal();

		void presetFromFile(const std::filesystem::path& sourceFile);

	protected:

		void buildContent(std::shared_ptr<Silica::SVerticalBox> contentBox) override;
		void validate() override;
		void onConfirm() override;

	private:

		std::string m_name;
		std::string m_sourcePath;
		std::string m_outputPath;

		int m_importFormat = 0;
		const std::vector<std::string> m_formatNames = { "MP3", "WAV", "OGG" };

		int m_loadType = 0;
		AudioClip::Mode m_types[2] = { AudioClip::Mode::Stream, AudioClip::Mode::Memory };
		const std::vector<std::string> m_typesNames = { "Stream", "Memory" };

		void resetInputs();

	};

}
