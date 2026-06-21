#pragma once

#include "SWidget.h"

namespace Silica {

	enum class HorizontalAlign { Fill, Left, Center, Right };
	enum class VerticalAlign { Fill, Top, Center, Bottom };

	class SAlign : public SWidget {
	public:

		struct Args {
			HorizontalAlign horizontalAlign = HorizontalAlign::Center;
			VerticalAlign verticalAlign = VerticalAlign::Center;
			WidgetPtr child = nullptr;
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

		HorizontalAlign m_horizontalAlign = HorizontalAlign::Center;
		VerticalAlign m_verticalAlign = VerticalAlign::Center;
		WidgetPtr m_child = nullptr;

	};

}
