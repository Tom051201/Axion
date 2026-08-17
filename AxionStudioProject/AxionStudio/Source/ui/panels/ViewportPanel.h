#pragma once

#include <functional>
#include <filesystem>
#include <memory>

#include <Silica/include/SWidget.h>

#include "AxionStudio/Source/EditorLayer.h"
#include "AxionStudio/Source/core/EditorCamera.h"

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

		void setup(EditorState* currentState, EditorState* prePauseState, int* stepFrames, EditorCamera* camera);
		void setViewportTexture(Silica::TextureID texID, Silica::Vec2 size);
		void setStatsText(const std::string& text);
		Silica::Vec2 getViewportSize() const;
		Silica::Vec2 getViewportPosition() const;
		bool isHovered(const Silica::Vec2& mousePos) const;
		void refreshToolbar();

		void setCallbacks(std::function<void()> onPlay, std::function<void()> onSimulate, std::function<void()> onStop);
		void setSkyboxDropCallback(std::function<void(const std::filesystem::path&)> callback) { m_onSkyboxDropped = callback; }
		void setSceneDropCallback(std::function<void(const std::filesystem::path&)> callback) { m_onSceneDropped = callback; }
		void setPrefabDropCallback(std::function<void(const std::filesystem::path&, Silica::Vec2)> callback) { m_onPrefabDropped = callback; }
		void setVisualScriptDropCallback(std::function<void(const std::filesystem::path&)> callback) { m_onVisualScriptDropped = callback; }

	private:

		EditorState* m_currentState = nullptr;
		EditorState* m_prePauseState = nullptr;
		int* m_stepFrames = nullptr;
		EditorCamera* m_camera = nullptr;

		std::function<void()> m_onPlay;
		std::function<void()> m_onSimulate;
		std::function<void()> m_onStop;
		std::function<void(const std::filesystem::path&)> m_onSkyboxDropped;
		std::function<void(const std::filesystem::path&)> m_onSceneDropped;
		std::function<void(const std::filesystem::path&, Silica::Vec2)> m_onPrefabDropped;
		std::function<void(const std::filesystem::path&)> m_onVisualScriptDropped;

		std::shared_ptr<Silica::SBox> m_toolbarContainer;
		std::shared_ptr<Silica::SBox> m_viewportContainer;
		std::shared_ptr<Silica::SImage> m_viewportImage;
		std::shared_ptr<Silica::STextBlock> m_statsText;

		void rebuildToolbar();

	};

}
