#pragma once

#include "AxionEngine/Source/render/Shader.h"

#include "AxionStudio/Source/core/Modal.h"

#include <string>

namespace Axion {

	class ShaderImportModal : public Modal {
	public:

		ShaderImportModal(const char* name) : Modal(name) {}
		~ShaderImportModal() override = default;

	private:

		void renderContent() override;
		void resetInputs() override;

		std::string m_name;
		std::string m_sourcePath;
		std::string m_outputPath;

		int m_formatIndex = 0;
		const char* m_formats[2] = { "HLSL", "GLSL" };

		int m_batchTexturesCount = 1;

	};

}
