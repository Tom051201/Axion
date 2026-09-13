#pragma once

#include <string>

#include "AxionStudio/Source/ui/ModalBase.h"

namespace Axion {

	class SkyboxImportModal : public ModalBase {
	public:

		SkyboxImportModal();

	protected:

		void buildContent(std::shared_ptr<Silica::SVerticalBox> contentBox) override;
		void validate() override;
		void onConfirm() override;

	private:

		std::string m_name;
		std::string m_texturePath;
		std::string m_pipelinePath;
		std::string m_outputPath;

		void resetInputs();

	};

}
