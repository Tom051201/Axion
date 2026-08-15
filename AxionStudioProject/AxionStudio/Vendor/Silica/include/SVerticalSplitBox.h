#pragma once

#include "SWidget.h"
#include "Renderer.h"

namespace Silica {

	class SVerticalSplitBox : public SWidget {
	public:

		struct Args {
			float topHeight = 250.0f;
			float splitterThickness = 4.0f;
			WidgetPtr topContent;
			WidgetPtr bottomContent;
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

		float getTopHeight() const { return m_topHeight; }

	private:

		float m_topHeight = 250.0f;
		float m_splitterThickness = 4.0f;

		bool m_isDraggingSplitter = false;
		bool m_isHoveredSplitter = false;
		float m_dragStartY = 0.0f;
		float m_initialTopHeight = 0.0f;

		WidgetPtr m_top;
		WidgetPtr m_bottom;

		Rect getSplitterRect(const Geometry& geo) const;

	};

}
