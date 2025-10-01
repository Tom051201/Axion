#pragma once

#include "AxionStudio/Source/core/Modal.h"
#include "AxionEngine/Source/audio/AudioClip.h"

namespace Axion {

	class AudioImportModal : public Modal {
	public:

		AudioImportModal(const char* name);
		~AudioImportModal() override;

		void close() override;

	private:

		void renderContent() override;

		void clearBuffers();

		char m_nameBuffer[128] = "";
		char m_sourcePathBuffer[256] = "";
		char m_outputPathBuffer[256] = "";

		int m_loadType = 0;
		AudioClip::Mode m_types[2] = { AudioClip::Mode::Stream, AudioClip::Mode::Memory };
		const char* m_typesNames[2] = { "Stream", "Memory" };

	};

}
