#pragma once

#include <functional>

#include "SWidget.h"

namespace Silica {

	class SMenuAnchor : public SWidget {
	public:

		struct Args {
			bool openOnHover = false;
			bool openOnRightClick = false;
			bool openToRight = false;
			bool showArrow = false;
			bool openAtMousePos = false;
			WidgetPtr anchorContent = nullptr;
			WidgetPtr menuContent = nullptr;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) override;
		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseButtonUp(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;

		void closeMenu();
		bool isOpen() const;

	private:

		WidgetPtr m_anchorContent;
		WidgetPtr m_menuContent;

		bool m_isOpen = false;
		bool m_openOnHover = false;
		bool m_openOnRightClick = false;
		bool m_openToRight = false;
		bool m_showArrow = false;
		bool m_openAtMousePos = false;
		Vec2 m_clickPos;

		Geometry m_menuGeometry;

		void drawTriangle(DrawList& drawList, const Vec2& center, float radius, Color color) const;

	};

}
