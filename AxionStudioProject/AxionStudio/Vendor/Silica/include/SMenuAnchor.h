#pragma once

#include <functional>
#include <optional>

#include "SWidget.h"
#include "MathTypes.h"
#include "OverlayManager.h"

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
			std::optional<std::string> hoverGroup;
			OverlayID parentOverlayID = 0;
			WidgetPtr anchorContent = nullptr;
			WidgetPtr menuContent = nullptr;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		void setRenderScale(float scale) override;

		EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) override;
		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseButtonUp(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseWheel(const Geometry& geom, const Vec2& pos, float delta) override;
		EventReply onDragOver(const Geometry& allocatedGeometry, const Vec2& mousePos, const DragDropPayload& payload) override;
		EventReply onDrop(const Geometry& allocatedGeometry, const Vec2& mousePos, const DragDropPayload& payload) override;

		void closeMenu();
		bool isOpen() const;

		void setParentMenu(SMenuAnchor* parentMenu);
		OverlayID getOverlayID() const;

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
		std::string m_hoverGroup = "";
		Vec2 m_clickPos;
		OverlayID m_overlayId = 0;
		OverlayID m_parentOverlayID = 0;
		SMenuAnchor* m_parentMenu = nullptr;
		SMenuAnchor* m_activeChild = nullptr;

		Geometry m_menuGeometry;

		void activateChild(SMenuAnchor* child);
		void drawTriangle(DrawList& drawList, const Vec2& center, float radius, Color color) const;

	};

}
