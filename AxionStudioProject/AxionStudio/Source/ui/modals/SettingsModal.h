#pragma once

#include <functional>

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"

namespace Axion {

	class SettingsModal {
	public:

		enum class Tab { EditorPreferences, FilePaths };

		SettingsModal() = default;
		~SettingsModal() = default;

		Silica::WidgetPtr getWidget(const std::vector<std::string>& currentPaths, std::function<void(std::vector<std::string>)> onApply, std::function<void()> onClose);

	private:

		void rebuildUI();
		void rebuildUI_Internal();

		// -- Tab Builders --
		Silica::WidgetPtr buildEditorPreferencesTab();
		Silica::WidgetPtr buildFilePathsTab();

		// -- Silica --
		std::shared_ptr<Silica::SBox> m_uiRoot;
		std::function<void()> m_onClose;
		bool m_rebuildQueued = false;

		// -- State --
		Tab m_activeTab = Tab::EditorPreferences;
		std::string m_budgetText;
		std::vector<std::string> m_libraryPaths;

		// -- Callbacks --
		std::function<void(std::vector<std::string>)> m_onApply;

	};

}
