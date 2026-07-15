#pragma once

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"

#include <string>
#include <filesystem>
#include <functional>

namespace Axion {

	class Texture2DImportModal {
	public:

		Texture2DImportModal() { resetInputs(); }
		~Texture2DImportModal() = default;

		Silica::WidgetPtr getWidget(std::function<void()> onClose);

	private:

		void rebuildUI();
		void rebuildUI_Internal();
		void resetInputs();

		std::string m_name;
		std::string m_sourcePath;
		std::string m_outputPath;

		int m_importType = 0;
		const char* m_types[3] = { "PNG", "JPG", "JPEG" };

		// -- Silica --
		std::shared_ptr<Silica::SBox> m_uiRoot;
		std::function<void()> m_onClose;
		bool m_rebuildQueued = false;

	};

}
