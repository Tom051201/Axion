#pragma once

#include <functional>
#include <optional>

#include "SWidget.h"

namespace Silica {

	class SSliderInt : public SWidget {
	public:

		struct Args {
			int initialValue = 0;
			int minValue = 0;
			int maxValue = 10;
			std::optional<Color> trackColor;
			std::optional<Color> fillColor;
			std::optional<Color> thumbColor;
			std::optional<Color> thumbDraggingColor;
			std::function<void(int)> onValueChanged = nullptr;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) override;
		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseButtonUp(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;

	private:

		int m_value = 0;
		int m_min = 0;
		int m_max = 10;
		bool m_isDragging = false;

		Color m_trackColor;
		Color m_fillColor;
		Color m_thumbColor;
		Color m_thumbDraggingColor;

		std::function<void(int)> m_onValueChanged;

		void updateValueFromMouse(float mouseX);
		void addRectToDrawList(DrawList& drawList, const Geometry& geo, Color color) const;

	};

}
