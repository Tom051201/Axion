#pragma once

#include <string>
#include <vector>

#include "SWidget.h"
#include "FontAtlas.h"

namespace Silica {

	class STextBlock;
	class SEditableText;
	class SVerticalBox;

	class SComboBox : public SWidget {
	public:

		struct Args {
			std::vector<std::string> options;
			std::string initialValue;
			bool searchable = false;
			FontAtlas* font = nullptr;
			std::function<void(std::string)> onValueChanged = nullptr;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		void setRenderScale(float scale) override;

		EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) override;
		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseButtonUp(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseWheel(const Geometry& allocatedGeometry, const Vec2& mousePos, float scrollDelta) override;

	private:

		void rebuildOptions(const std::string& query);

		WidgetPtr m_mainButton;
		WidgetPtr m_popupMenu;

		std::shared_ptr<STextBlock> m_textBlock;
		std::shared_ptr<SEditableText> m_searchBox;
		std::shared_ptr<SVerticalBox> m_optionsBox;

		std::vector<std::string> m_options;
		FontAtlas* m_font = nullptr;
		std::function<void(std::string)> m_onValueChanged;

		std::string m_currentValue;
		bool m_isOpen = false;

	};

}
