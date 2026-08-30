#pragma once

#include <string>
#include <memory>

#include <Silica/include/SWidget.h>

namespace Silica {
	class SBox;
	class STextBlock;
	class SButton;
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
		void validate();

		std::string m_exportPath;
		bool m_openAfterExport = true;

		// -- Silica --
		std::shared_ptr<Silica::SBox> m_uiRoot;
		std::shared_ptr<Silica::STextBlock> m_validationText;
		std::shared_ptr<Silica::SButton> m_exportBtn;
		bool m_rebuildQueued = false;

	};

}
