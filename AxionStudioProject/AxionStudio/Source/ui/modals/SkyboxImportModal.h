#pragma once

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/FontAtlas.h"

#include <string>
#include <filesystem>
#include <functional>

namespace Axion {

	class SkyboxImportModal {
	public:

		SkyboxImportModal() { resetInputs(); }
		~SkyboxImportModal() = default;

		Silica::WidgetPtr getWidget(Silica::FontAtlas* font, std::function<void()> onClose);

	private:

		void rebuildUI();
		void rebuildUI_Internal();
		void resetInputs();

		std::string m_name;
		std::string m_texturePath;
		std::string m_pipelinePath;
		std::string m_outputPath;

		// -- Silica --
		std::shared_ptr<Silica::SBox> m_uiRoot;
		Silica::FontAtlas* m_font = nullptr;
		std::function<void()> m_onClose;
		bool m_rebuildQueued = false;

	};

}
