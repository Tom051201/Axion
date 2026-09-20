#pragma once

#include <functional>
#include <filesystem>
#include <memory>

#include <Silica/include/SWidget.h>

#include "AxionStudio/Source/EditorLayer.h"
#include "AxionStudio/Source/core/EditorCamera.h"
#include "AxionStudio/Source/core/TransformGizmo.h"

#include "AxionEngine/Source/events/Event.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"

namespace YAML {
	class Node;
	class Emitter;
}

namespace Silica {
	class SBox;
	class SImage;
	class STextBlock;
}

namespace Axion {

	class ViewportPanel {
	public:

		ViewportPanel() = default;
		~ViewportPanel() = default;

		Silica::WidgetPtr getWidget();

		void onEvent(Event& ev);

		void setup(EditorState* currentState, EditorState* prePauseState, int* stepFrames, EditorCamera* camera, TransformGizmo* gizmo);
		void setViewportTexture(Silica::TextureID texID, Silica::Vec2 size);
		void setStatsText(const std::string& text);
		Silica::Vec2 getViewportSize() const;
		Silica::Vec2 getViewportPosition() const;
		Silica::Vec2 getRelativeMousePos() const;
		bool isHovered(const Silica::Vec2& mousePos) const;
		void refreshToolbar();
		void refresh();

		void setEventCallback(std::function<void(Event&)> callback) { m_eventCallback = callback; }
		void setPrefabDropCallback(std::function<void(const std::filesystem::path&, Silica::Vec2)> callback) { m_onPrefabDropped = callback; }
		void setVisualScriptDropCallback(std::function<void(const std::filesystem::path&)> callback) { m_onVisualScriptDropped = callback; }
		void setRequestViewportTextureCallback(const std::function<Silica::TextureID()> callback) { m_requestViewportTexture = callback; }

		void loadSettings(const YAML::Node& editorConfig);
		void saveSettings(YAML::Emitter& out) const;

	private:

		void rebuildUI();
		void rebuildUI_Internal();
		void rebuildToolbar();

		EditorState* m_currentState = nullptr;
		EditorState* m_prePauseState = nullptr;
		int* m_stepFrames = nullptr;
		EditorCamera* m_camera = nullptr;
		TransformGizmo* m_gizmo = nullptr;

		std::function<void(Event&)> m_eventCallback;
		std::function<void()> m_onPlay;
		std::function<void()> m_onSimulate;
		std::function<void()> m_onStop;
		std::function<void(const std::filesystem::path&, Silica::Vec2)> m_onPrefabDropped;
		std::function<void(const std::filesystem::path&)> m_onVisualScriptDropped;
		std::function<Silica::TextureID()> m_requestViewportTexture;

		std::shared_ptr<Silica::SBox> m_uiRoot;
		bool m_rebuildQueued = false;

		std::shared_ptr<Silica::SBox> m_toolbarContainer;
		std::shared_ptr<Silica::SBox> m_viewportContainer;
		std::shared_ptr<Silica::SImage> m_viewportImage;
		std::shared_ptr<Silica::STextBlock> m_statsText;

		EventReply onProjectChanged(ProjectChangedEvent& ev);
		EventReply onEditorSettingsChanged(EditorSettingsChangedEvent& ev);

	};

}
