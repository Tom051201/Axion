#pragma once

#include <string>
#include <vector>

#include "AxionStudio/Source/ui/ModalBase.h"

namespace Axion {

	class ShaderImportModal : public ModalBase {
	public:

		ShaderImportModal();

	protected:

		void buildContent(std::shared_ptr<Silica::SVerticalBox> contentBox) override;
		void validate() override;
		void onConfirm() override;

	private:

		std::string m_name;
		std::string m_sourcePath;
		std::string m_outputPath;

		int m_formatIndex = 0;
		const std::vector<std::string> m_formats = { "HLSL", "GLSL" };

		int m_batchTexturesCount = 1;

		void resetInputs();

	};

}
