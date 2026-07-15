#pragma once

#include <functional>
#include <optional>

#include "SWidget.h"
#include "MathTypes.h"

namespace Silica {

	class SMenuAnchor : public SWidget {
	public:

		struct Args {
			bool openOnHover = false;
			bool openOnRightClick = false;
			bool openToRight = false;
			bool showArrow = false;
			bool openAtMousePos = false;
			std::optional<Color> arrowNormal;
			std::optional<Color> arrowHover;
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
		EventReply onMouseWheel(const Geometry& geom, const Vec2& pos, float delta) override;
		EventReply onDragOver(const Geometry& allocatedGeometry, const Vec2& mousePos, const DragDropPayload& payload) override;
		EventReply onDrop(const Geometry& allocatedGeometry, const Vec2& mousePos, const DragDropPayload& payload) override;

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
		Color m_arrowNormal;
		Color m_arrowHover;
		Vec2 m_clickPos;

		Geometry m_menuGeometry;

		void drawTriangle(DrawList& drawList, const Vec2& center, float radius, Color color) const;

	};

}
