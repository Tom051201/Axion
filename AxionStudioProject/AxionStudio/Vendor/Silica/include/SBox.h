#pragma once

#include <cstdint>
#include <optional>

#include "SWidget.h"
#include "Renderer.h"
#include "MathTypes.h"

namespace Silica {

	class SBox : public SWidget {
	public:

		struct Args {
			Vec2 padding = Vec2::zero();
			std::optional<Color> backgroundColor;
			std::optional<Color> hoverColor;
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
		EventReply onMouseWheel(const Geometry& allocatedGeometry, const Vec2& mousePos, float scrollDelta) override;

	private:

		Vec2 m_padding;
		Color m_backgroundColor;
		Color m_hoverColor;
		std::function<EventReply()> m_onClick;
		WidgetPtr m_child;

		void addRectToDrawList(DrawList& drawList, const Geometry& geo, Color color) const;

	};

}
