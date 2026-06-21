#pragma once

#include "AxionEngine/Source/core/Version.h"
#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/FontAtlas.h"

#include <string>
#include <filesystem>
#include <functional>

namespace Axion {

	class CreateProjectModal {
	public:

		CreateProjectModal() { resetInputs(); }
		~CreateProjectModal() = default;

		Silica::WidgetPtr getWidget(Silica::FontAtlas* font);

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
		Silica::FontAtlas* m_font = nullptr;
		bool m_rebuildQueued = false;

	};

}
