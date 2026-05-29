#pragma once

#include <optional>

#include "SWidget.h"
#include "Renderer.h"

namespace Silica {

	class SButton : public SWidget {
	public:

		struct Args {
			Vec2 padding = { 10.0f, 10.0f };
			std::optional<Color> color;
			std::optional<Color> hoverColor;
			std::optional<Color> pressedColor;
			std::function<EventReply()> onClick = nullptr;
			WidgetPtr child = nullptr;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) override;
		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseButtonUp(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;

	private:

		Vec2 m_padding;
		Color m_color;
		Color m_hoverColor;
		Color m_pressedColor;
		std::function<EventReply()> m_onClick;
		WidgetPtr m_child;

		bool m_isPressed = false;

		void addRectToDrawList(DrawList& drawList, const Geometry& geo, Color color) const;

	};

}
