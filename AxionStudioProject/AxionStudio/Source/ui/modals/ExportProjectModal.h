#pragma once

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/FontAtlas.h"

#include <string>
#include <filesystem>
#include <functional>

namespace Axion {

	class ExportProjectModal {
	public:

		ExportProjectModal() = default;
		~ExportProjectModal() = default;

		Silica::WidgetPtr getWidget(Silica::FontAtlas* font);

	private:

		void rebuildUI();
		void rebuildUI_Internal();

		std::string m_exportPath;
		bool m_openAfterExport = true;

		// -- Silica --
		std::shared_ptr<Silica::SBox> m_uiRoot;
		Silica::FontAtlas* m_font = nullptr;
		bool m_rebuildQueued = false;

	};

}
