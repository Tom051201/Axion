#pragma once

#include "AxionStudio/Source/core/Modal.h"

#include <filesystem>
#include <string>

namespace Axion {

	class MeshImportModal : public Modal {
	public:

		MeshImportModal(const char* name) : Modal(name) {}
		~MeshImportModal() override = default;

		void presetFromFile(const std::filesystem::path& sourceFile);

	private:

		void renderContent() override;
		void resetInputs() override;


		std::string m_name;
		std::string m_sourcePath;
		std::string m_outputPath;

		int m_importType = 0;
		const char* m_types[1] = { "OBJ" };

	};

}
