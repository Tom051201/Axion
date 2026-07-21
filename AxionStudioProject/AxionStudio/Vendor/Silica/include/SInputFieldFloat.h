#pragma once

#include <string>

#include "SWidget.h"
#include "FontAtlas.h"

namespace Silica {

	class SInputFieldFloat : public SWidget {
	public:

		struct Args {
			float initialValue = 0.0f;
			FontAtlas* font = nullptr;
			std::function<void(float)> onValueChanged = nullptr;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		void setRenderScale(float scale) override;

		void setValue(float newValue, bool moveCursorToEnd = false);

		EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) override;
		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseButtonUp(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseWheel(const Geometry& allocatedGeometry, const Vec2& mousePos, float scrollDelta) override;
		EventReply onDragOver(const Geometry& allocatedGeometry, const Vec2& mousePos, const DragDropPayload& payload) override;
		EventReply onDrop(const Geometry& allocatedGeometry, const Vec2& mousePos, const DragDropPayload& payload) override;

	private:

		WidgetPtr m_editableText;
		float m_currentValue;
		std::function<void(float)> m_onValueChanged;

	};

}
