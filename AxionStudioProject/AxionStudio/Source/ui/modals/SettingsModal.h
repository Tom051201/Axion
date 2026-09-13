#pragma once

#include <functional>
#include <vector>
#include <string>

#include <Silica/include/SWidget.h>

namespace Silica {
	class SBox;
}

namespace Axion {

	struct SettingsPayload {
		uint32_t maxAssetsPerFrame = 2;
		bool enableDiscordRPC = true;
		std::vector<std::string> assetLibraryPaths;
		bool contentBrowserShowContentArea = true;
		bool contentBrowserShowVFSTree = true;
		bool contentBrowserShowPhysicalTree = true;
		bool materialEditorInvertCamera = false;
	};

	class SettingsModal {
	public:

		enum class Tab { EditorPreferences, FilePaths, Panels };

		SettingsModal() = default;
		~SettingsModal() = default;

		Silica::WidgetPtr getWidget(const SettingsPayload& initialSettings, std::function<void(const SettingsPayload&)> onApply, std::function<void()> onClose);

	private:

		void rebuildUI();
		void rebuildUI_Internal();

		// -- Tab Builders --
		Silica::WidgetPtr buildEditorPreferencesTab();
		Silica::WidgetPtr buildFilePathsTab();
		Silica::WidgetPtr buildPanelsTab();

		// -- Silica --
		std::shared_ptr<Silica::SBox> m_uiRoot;
		std::function<void()> m_onClose;
		bool m_rebuildQueued = false;

		// -- State --
		Tab m_activeTab = Tab::EditorPreferences;
		SettingsPayload m_workingSettings;

		// -- Callbacks --
		std::function<void(const SettingsPayload&)> m_onApply;

	};

}
