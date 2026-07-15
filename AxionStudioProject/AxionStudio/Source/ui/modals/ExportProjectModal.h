#pragma once

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"

#include <string>
#include <filesystem>
#include <functional>

namespace Axion {

	class ExportProjectModal {
	public:

		ExportProjectModal() = default;
		~ExportProjectModal() = default;

		Silica::WidgetPtr getWidget();

	private:

		void rebuildUI();
		void rebuildUI_Internal();

		std::string m_exportPath;
		bool m_openAfterExport = true;

		// -- Silica --
		std::shared_ptr<Silica::SBox> m_uiRoot;
		bool m_rebuildQueued = false;

	};

}
