#pragma once

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"

#include <filesystem>
#include <string>
#include <functional>

namespace Axion {

	class MeshImportModal {
	public:

		MeshImportModal() { resetInputs(); }
		~MeshImportModal() = default;

		void presetFromFile(const std::filesystem::path& sourceFile);

		Silica::WidgetPtr getWidget(std::function<void()> onClose);

	private:

		void rebuildUI();
		void rebuildUI_Internal();
		void resetInputs();

		std::string m_name;
		std::string m_sourcePath;
		std::string m_outputPath;

		int m_importType = 0;
		const char* m_types[3] = { "OBJ", "GLTF", "GLB" };

		// -- Silica --
		std::shared_ptr<Silica::SBox> m_uiRoot;
		std::function<void()> m_onClose;
		bool m_rebuildQueued = false;

	};

}
