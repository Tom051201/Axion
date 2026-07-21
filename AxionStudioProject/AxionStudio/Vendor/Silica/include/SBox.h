#pragma once

#include <cstdint>
#include <optional>

#include "SWidget.h"
#include "Renderer.h"
#include "MathTypes.h"
#include "Geometry.h"

namespace Silica {

	class SBox : public SWidget {
	public:

		struct Args {
			Vec2 padding = Vec2::zero();
			std::optional<Vec2> explicitSize;
			float borderThickness = 0.0f;
			bool consumePointerEvents = false;
			std::optional<Color> backgroundColor;
			std::optional<Color> borderColor;
			std::function<EventReply(const DragDropPayload&)> onDragOver = nullptr;
			std::function<EventReply(const DragDropPayload&)> onDrop = nullptr;
			WidgetPtr child = nullptr;
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
		EventReply onDragOver(const Geometry& allocatedGeometry, const Vec2& mousePos, const DragDropPayload& payload) override;
		EventReply onDrop(const Geometry& allocatedGeometry, const Vec2& mousePos, const DragDropPayload& payload) override;

		void setChild(WidgetPtr child);

	private:

		Vec2 m_padding;
		Vec2 m_explicitSize = Vec2::zero();
		float m_borderThickness = 0.0f;
		bool m_consumePointerEvents = false;
		Color m_backgroundColor;
		Color m_borderColor;
		std::function<EventReply(const DragDropPayload&)> m_onDragOver;
		std::function<EventReply(const DragDropPayload&)> m_onDrop;
		WidgetPtr m_child;

		Geometry getChildGeometry(const Geometry& allocatedGeometry) const;

	};

}
