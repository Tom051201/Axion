#pragma once

#include <functional>

#include "SWidget.h"
#include "Theme.h"
#include "Renderer.h"

namespace Silica {

	class SColorPicker : public SWidget {
	public:

		struct Args {
			Color initialColor = Color::white();
			std::function<void(Color)> onColorChanged = nullptr;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) override;
		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseButtonUp(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;

	private:

		// -- HSV Format --
		float m_h = 0.0f;
		float m_s = 1.0f;
		float m_v = 1.0f;
		float m_a = 1.0f;

		enum class DragState { None, SVBox, HueBar, AlphaBar };
		DragState m_dragState = DragState::None;

		std::function<void(Color)> m_onColorChanged;

		Color hsvToRgb(float h, float s, float v, float a) const;
		void rgbToHsv(Color c, float& h, float& s, float& v, float& a) const;

		void updateFromMouse(const Geometry& allocatedGeometry, const Vec2& mousePos);

		Rect getSVBoxRect(const Geometry& geo) const;
		Rect getHueBarRect(const Geometry& geo) const;
		Rect getAlphaBarRect(const Geometry& geo) const;

		void addGradientRect(DrawList& drawList, const Geometry& geo, Color tl, Color tr, Color br, Color bl) const;
		void addRectToDrawList(DrawList& drawList, const Geometry& geo, Color color) const;

	};

}
