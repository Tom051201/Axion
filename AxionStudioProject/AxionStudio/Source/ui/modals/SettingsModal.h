#pragma once

#include <functional>
#include <vector>
#include <string>

#include <Silica/include/SWidget.h>

#include "AxionEngine/Source/events/Event.h"

#include "AxionStudio/Source/core/EditorEvents.h"

namespace Silica {
	class SBox;
}

namespace Axion {

	class SettingsModal {
	public:

		enum class Tab { EditorPreferences, Graphics, FilePaths, Panels };

		SettingsModal() = default;
		~SettingsModal() = default;

		Silica::WidgetPtr getWidget(std::function<void()> onClose);
		void setEventCallback(std::function<void(Event&)> callback) { m_eventCallback = callback; }

	private:

		struct WorkingState {
			uint32_t maxAssetsPerFrame = 2;
			bool enableDiscordRPC = true;
			std::vector<std::string> assetLibraryPaths;
			bool contentBrowserShowContentArea = true;
			bool contentBrowserShowVFSTree = true;
			bool contentBrowserShowPhysicalTree = true;
			bool materialEditorInvertCamera = false;
			bool viewportPanelShowRendererStats = true;
			bool viewportPanelInvertCameraX = false;
			bool viewportPanelInvertCameraY = false;
			float viewportPanelGizmoScale = 1.0f;
		};

		void rebuildUI();
		void rebuildUI_Internal();

		// -- Tab Builders --
		Silica::WidgetPtr buildEditorPreferencesTab();
		Silica::WidgetPtr buildGraphicsTab();
		Silica::WidgetPtr buildFilePathsTab();
		Silica::WidgetPtr buildPanelsTab();

		// -- Silica --
		std::shared_ptr<Silica::SBox> m_uiRoot;
		std::function<void()> m_onClose;
		bool m_rebuildQueued = false;

		// -- State --
		Tab m_activeTab = Tab::EditorPreferences;
		WorkingState m_workingState;
		EditorSettingType m_pendingChanges = EditorSettingType::None;
		bool m_showCancelPopup = false;

		// -- Callbacks --
		std::function<void(Event&)> m_eventCallback;

	};

}
