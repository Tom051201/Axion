#pragma once

#include <vector>

#include "SWidget.h"

namespace Silica {

	class SOverlay : public SWidget {
	public:

		struct Args {
			std::vector<WidgetPtr> children;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) override;
		EventReply onMouseButtonUp(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseWheel(const Geometry& allocatedGeometry, const Vec2& mousePos, float scrollDelta) override;

		void addChild(WidgetPtr child);
		void removeChild(WidgetPtr child);

	private:

		std::vector<WidgetPtr> m_children;

	};

}
