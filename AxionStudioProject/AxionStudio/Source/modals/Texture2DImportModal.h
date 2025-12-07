#pragma once

#include "AxionStudio/Source/core/Modal.h"

namespace Axion {

	class Texture2DImportModal : public Modal {
	public:

		Texture2DImportModal(const char* name);
		~Texture2DImportModal() override;

		void close() override;

	private:

		void renderContent() override;

		void clearBuffers();

		char m_nameBuffer[128] = "";
		char m_sourcePathBuffer[256] = "";
		char m_outputPathBuffer[256] = "";

		int m_importType = 0;
		const char* m_types[1] = { "PNG" };

	};

}
