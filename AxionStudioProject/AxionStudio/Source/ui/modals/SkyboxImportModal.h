#pragma once

#include <string>
#include <memory>
#include <functional>

#include <Silica/include/SWidget.h>

namespace Silica {
	class SBox;
}

namespace Axion {

	class SkyboxImportModal {
	public:

		SkyboxImportModal() { resetInputs(); }
		~SkyboxImportModal() = default;

		Silica::WidgetPtr getWidget(std::function<void()> onClose);

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
		std::function<void()> m_onClose;
		bool m_rebuildQueued = false;

	};

}
