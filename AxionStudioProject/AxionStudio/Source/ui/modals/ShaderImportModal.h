#pragma once

#include <string>
#include <memory>
#include <functional>

#include <Silica/include/SWidget.h>

namespace Silica {
	class SBox;
}

namespace Axion {

	class ShaderImportModal {
	public:

		ShaderImportModal() { resetInputs(); }
		~ShaderImportModal() = default;

		Silica::WidgetPtr getWidget(std::function<void()> onClose);

	private:

		void rebuildUI();
		void rebuildUI_Internal();
		void resetInputs();

		std::string m_name;
		std::string m_sourcePath;
		std::string m_outputPath;

		int m_formatIndex = 0;
		const char* m_formats[2] = { "HLSL", "GLSL" };

		int m_batchTexturesCount = 1;

		// -- Silica --
		std::shared_ptr<Silica::SBox> m_uiRoot;
		std::function<void()> m_onClose;
		bool m_rebuildQueued = false;

	};

}
