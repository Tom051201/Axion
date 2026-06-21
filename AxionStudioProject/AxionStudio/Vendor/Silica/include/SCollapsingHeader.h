#pragma once

#include <string>
#include <optional>

#include "Renderer.h"
#include "SWidget.h"
#include "FontAtlas.h"

namespace Silica {

	class SCollapsingHeader : public SWidget {
	public:

		struct Args {
			std::string title = "Header";
			bool initiallyOpen = true;
			std::optional<Color> headerColor;
			std::optional<Color> headerHoverColor;
			std::optional<Color> textColor;
			FontAtlas* font = nullptr;
			WidgetPtr trailingWidget = nullptr;
			WidgetPtr content = nullptr;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) override;
		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseButtonUp(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseWheel(const Geometry& allocatedGeometry, const Vec2& mousePos, float scrollDelta) override;

	private:

		std::string m_title;
		bool m_isOpen = true;
		WidgetPtr m_content;
		FontAtlas* m_font = nullptr;
		WidgetPtr m_trailingWidget;

		Color m_headerColor;
		Color m_headerHoverColor;
		Color m_textColor;

		float m_headerHeight = 24.0f;
		bool m_isHeaderHovered = false;

		Rect getHeaderRect() const;
		void addRectToDrawList(DrawList& drawList, const Geometry& geo, Color color) const;
		void drawTriangle(DrawList& drawList, const Vec2& center, float radius, bool isOpen, Color color) const;

	};

}
