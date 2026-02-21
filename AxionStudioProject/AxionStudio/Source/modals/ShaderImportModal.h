#pragma once

#include "AxionEngine/Source/render/Shader.h"

#include "AxionStudio/Source/core/Modal.h"

namespace Axion {

	class ShaderImportModal : public Modal {
	public:

		ShaderImportModal(const char* name);
		~ShaderImportModal() override;

		void close() override;

	private:

		void renderContent() override;

		void clearBuffers();

		char m_nameBuffer[128] = "";
		char m_sourcePathBuffer[256] = "";
		char m_outputPathBuffer[256] = "";

		int m_formatIndex = 0;
		const char* m_formats[2] = { ".hlsl", ".glsl" };

		int m_batchTexturesCount = 1;

	};

}
