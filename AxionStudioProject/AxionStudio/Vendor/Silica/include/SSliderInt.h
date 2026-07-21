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
			int snapStep = 1;
			bool showText = true;
			std::string format = "%d";
			FontAtlas* font = nullptr;
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
		EventReply onMouseWheel(const Geometry& allocatedGeometry, const Vec2& mousePos, float scrollDelta) override;

	private:

		int m_value = 0;
		int m_min = 0;
		int m_max = 10;
		int m_snapStep = 1;
		bool m_isDragging = false;

		bool m_showText = true;
		std::string m_format;
		FontAtlas* m_font = nullptr;

		Color m_trackColor;
		Color m_fillColor;
		Color m_thumbColor;
		Color m_thumbDraggingColor;

		std::function<void(int)> m_onValueChanged;

		void updateValueFromMouse(float mouseX);

	};

}
