#pragma once

#include "AxionStudio/Source/core/Modal.h"

#include <string>

namespace Axion {

	class SkyboxImportModal : public Modal {
	public:

		SkyboxImportModal(const char* name) : Modal(name) {}
		~SkyboxImportModal() override = default;

	private:

		void renderContent() override;
		void resetInputs() override;

		std::string m_name;
		std::string m_texturePath;
		std::string m_pipelinePath;
		std::string m_outputPath;

	};

}
