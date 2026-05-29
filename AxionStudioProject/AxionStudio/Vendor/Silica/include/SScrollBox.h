#pragma once

#include <optional>

#include "SWidget.h"
#include "Renderer.h"

namespace Silica {

	class SScrollBox : public SWidget {
	public:

		struct Args {
			WidgetPtr child = nullptr;
			float scrollSpeed = 40.0f;
			std::optional<Color> thumbColor;
			std::optional<Color> thumbDraggingColor;
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

		WidgetPtr m_child;
		float m_scrollOffset = 0.0f;
		float m_scrollSpeed = 40.0f;
		float m_maxScroll = 0.0f;

		bool m_isDraggingThumb = false;
		float m_dragClickOffsetY = 0.0f;

		Color m_thumbColor;
		Color m_thumbDraggingColor;

		void addRectToDrawList(DrawList& drawList, const Geometry& geo, Color color) const;
		Rect getThumbRect(const Geometry& allocatedGeometry) const;

	};

}
