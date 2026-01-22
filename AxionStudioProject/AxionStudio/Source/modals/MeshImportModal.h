#pragma once

#include "AxionStudio/Source/core/Modal.h"

#include <filesystem>

namespace Axion {

	class MeshImportModal : public Modal {
	public:

		MeshImportModal(const char* name);
		~MeshImportModal() override;

		void close() override;

		void presetFromFile(const std::filesystem::path& sourceFile);

	private:

		void renderContent() override;

		void clearBuffers();

		char m_nameBuffer[128] = "";
		char m_sourcePathBuffer[256] = "";
		char m_outputPathBuffer[256] = "";

		int m_importType = 0;
		const char* m_types[1] = { "OBJ" };

	};

}
