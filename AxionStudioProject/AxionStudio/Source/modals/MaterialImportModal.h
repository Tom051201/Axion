#pragma once

#include "AxionStudio/Source/core/Modal.h"
#include "AxionEngine/Source/core/Math.h"

namespace Axion {

	class MaterialImportModal : public Modal {
	public:

		MaterialImportModal(const char* name);
		~MaterialImportModal() override;

		void close() override;

	private:

		void renderContent() override;

		void clearBuffers();

		char m_nameBuffer[128] = "";
		char m_sourcePathBuffer[256] = "";
		char m_outputPathBuffer[256] = "";

		Vec4 m_color = Vec4::one();

	};

}
