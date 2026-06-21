#pragma once

#include "AxionStudio/Source/EditorLayer.h"
#include "AxionStudio/Source/core/EditorCamera.h"

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/SImage.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/FontAtlas.h"

#include <functional>

namespace Axion {

	class ViewportPanel {
	public:

		ViewportPanel() = default;
		~ViewportPanel() = default;

		Silica::WidgetPtr getWidget(Silica::FontAtlas* font);

		void setup(EditorState* currentState, EditorState* prePauseState, int* stepFrames, EditorCamera* camera);
		void setViewportTexture(Silica::TextureID texID, Silica::Vec2 size);
		void setStatsText(const std::string& text);
		Silica::Vec2 getViewportSize() const;
		bool isHovered(const Silica::Vec2& mousePos) const;
		void refreshToolbar();

		void setCallbacks(std::function<void()> onPlay, std::function<void()> onSimulate, std::function<void()> onStop);

	private:

		void rebuildToolbar();

		EditorState* m_currentState = nullptr;
		EditorState* m_prePauseState = nullptr;
		int* m_stepFrames = nullptr;
		EditorCamera* m_camera = nullptr;

		Silica::FontAtlas* m_font = nullptr;

		std::function<void()> m_onPlay;
		std::function<void()> m_onSimulate;
		std::function<void()> m_onStop;

		std::shared_ptr<Silica::SBox> m_toolbarContainer;
		std::shared_ptr<Silica::SBox> m_viewportContainer;
		std::shared_ptr<Silica::SImage> m_viewportImage;
		std::shared_ptr<Silica::STextBlock> m_statsText;
	};

}
