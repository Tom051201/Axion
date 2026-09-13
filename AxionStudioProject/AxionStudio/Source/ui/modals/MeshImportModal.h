#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "AxionStudio/Source/ui/ModalBase.h"

namespace Axion {

	class MeshImportModal : public ModalBase {
	public:

		MeshImportModal();

		void presetFromFile(const std::filesystem::path& sourceFile);

	protected:

		void buildContent(std::shared_ptr<Silica::SVerticalBox> contentBox) override;
		void validate() override;
		void onConfirm() override;

	private:

		std::string m_name;
		std::string m_sourcePath;
		std::string m_outputPath;

		int m_importType = 0;
		const std::vector<std::string> m_types = { "OBJ", "GLTF", "GLB" };

		void resetInputs();

	};

}
