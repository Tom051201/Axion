#pragma once

#include <string>
#include <memory>

#include <Silica/include/SWidget.h>

#include "AxionEngine/Source/core/Version.h"

namespace Silica {
	class SBox;
}

namespace Axion {

	class CreateProjectModal {
	public:

		CreateProjectModal() { resetInputs(); }
		~CreateProjectModal() = default;

		Silica::WidgetPtr getWidget();

	private:

		void rebuildUI();
		void rebuildUI_Internal();
		void resetInputs();

		std::string m_name;
		std::string m_outputPath;
		std::string m_author;
		std::string m_company;
		std::string m_description;
		Version m_version = Version(1, 0, 0);

		// -- Silica --
		std::shared_ptr<Silica::SBox> m_uiRoot;
		bool m_rebuildQueued = false;

	};

}
