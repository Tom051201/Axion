#pragma once

#include "SWidget.h"
#include "Renderer.h"

namespace Silica {

	class SScissorBox : public SWidget {
	public:

		struct Args {
			WidgetPtr child = nullptr;
		};

		void construct(const Args& args) {
			m_child = args.child;
		}

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) override;
		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseButtonUp(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;

	private:

		WidgetPtr m_child;

	};

}
