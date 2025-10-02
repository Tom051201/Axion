#pragma once

#include "AxionStudio/Source/core/Modal.h"

namespace Axion {

	class SkyboxImportModal : public Modal {
	public:

		SkyboxImportModal(const char* name);
		~SkyboxImportModal() override;

		void close() override;

	private:

		void renderContent() override;

		void clearBuffers();

		char m_nameBuffer[128] = "";
		char m_sourcePathBuffer[256] = "";
		char m_outputPathBuffer[256] = "";
		char m_shaderPathBuffer[256] = "";

	};

}
