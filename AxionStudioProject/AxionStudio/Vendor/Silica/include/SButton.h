#pragma once

#include <optional>

#include "SWidget.h"
#include "Renderer.h"

namespace Silica {

	class SButton : public SWidget {
	public:

		struct Args {
			Vec2 padding = { 10.0f, 10.0f };
			bool enabled = true;
			std::optional<Color> color;
			std::optional<Color> hoverColor;
			std::optional<Color> pressedColor;
			std::optional<Color> disabledColor;
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

		void setEnabled(bool enabled);
		bool isEnabled() const;

	private:

		Vec2 m_padding;
		bool m_isEnabled = true;
		Color m_color;
		Color m_hoverColor;
		Color m_pressedColor;
		Color m_disabledColor;
		std::function<EventReply()> m_onClick;
		WidgetPtr m_child;

		bool m_isPressed = false;

		void addRectToDrawList(DrawList& drawList, const Geometry& geo, Color color) const;

	};

}
