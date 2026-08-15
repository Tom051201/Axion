#pragma once

#include "SWidget.h"
#include "Renderer.h"

namespace Silica {

	class SHorizontalSplitBox : public SWidget {
	public:

		struct Args {
			float leftWidth = 250.0f;
			float splitterThickness = 4.0f;
			WidgetPtr leftContent;
			WidgetPtr rightContent;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		void setRenderScale(float scale) override;

		EventReply onMouseMove(const Geometry& geo, const Vec2& pos) override;
		EventReply onMouseButtonDown(const Geometry& geo, const Vec2& pos, MouseButton btn) override;
		EventReply onMouseButtonUp(const Geometry& geo, const Vec2& pos, MouseButton btn) override;
		EventReply onMouseWheel(const Geometry& geo, const Vec2& pos, float delta) override;
		EventReply onDragOver(const Geometry& allocatedGeometry, const Vec2& mousePos, const DragDropPayload& payload) override;
		EventReply onDrop(const Geometry& allocatedGeometry, const Vec2& mousePos, const DragDropPayload& payload) override;

		float getLeftWidth() const { return m_leftWidth; }

	private:

		float m_leftWidth = 250.0f;
		float m_splitterThickness = 4.0f;

		bool m_isDraggingSplitter = false;
		bool m_isHoveredSplitter = false;
		float m_dragStartX = 0.0f;
		float m_initialLeftWidth = 0.0f;

		WidgetPtr m_left;
		WidgetPtr m_right;

		Rect getSplitterRect(const Geometry& geo) const;

	};

}
