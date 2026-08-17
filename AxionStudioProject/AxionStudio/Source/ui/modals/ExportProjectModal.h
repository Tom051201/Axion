#pragma once

#include <string>
#include <memory>

#include <Silica/include/SWidget.h>

namespace Silica {
	class SBox;
}

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
