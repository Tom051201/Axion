#pragma once

#include "AxionStudio/Source/core/Modal.h"

#include <string>

namespace Axion {

	class Texture2DImportModal : public Modal {
	public:

		Texture2DImportModal(const char* name) : Modal(name) {}
		~Texture2DImportModal() override = default;

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
