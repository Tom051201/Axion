#pragma once

#include "AxionStudio/Source/core/Modal.h"

#include <string>

namespace Axion {

	class TextureCubeImportModal : public Modal {
	public:

		TextureCubeImportModal(const char* name) : Modal(name) {}
		~TextureCubeImportModal() override = default;

	private:

		void renderContent() override;
		void resetInputs() override;

		std::string m_name;
		std::string m_sourcePath;
		std::string m_outputPath;

		int m_importType = 0;
		const char* m_types[3] = { "PNG", "JPG", "JPEG" };

	};

}
