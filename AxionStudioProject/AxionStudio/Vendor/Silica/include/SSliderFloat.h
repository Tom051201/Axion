#pragma once

#include <functional>
#include <optional>

#include "SWidget.h"

namespace Silica {

	class SSliderFloat : public SWidget {
	public:

		struct Args {
			float initialValue = 0.0f;
			float minValue = 0.0f;
			float maxValue = 1.0f;
			std::optional<Color> trackColor;
			std::optional<Color> fillColor;
			std::optional<Color> thumbColor;
			std::optional<Color> thumbDraggingColor;
			std::function<void(float)> onValueChanged = nullptr;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) override;
		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseButtonUp(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;

	private:

		float m_value = 0.0f;
		float m_min = 0.0f;
		float m_max = 0.0f;
		bool m_isDragging = false;

		Color m_trackColor;
		Color m_fillColor;
		Color m_thumbColor;
		Color m_thumbDraggingColor;

		std::function<void(float)> m_onValueChanged;

		void updateValueFromMouse(float mouseX);
		void addRectToDrawList(DrawList& drawList, const Geometry& geo, Color color) const;

	};

}
