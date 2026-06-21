#pragma once

#include "SWidget.h"

namespace Silica {

	class SWrapBox : public SWidget {
	public:

		struct Args {
			float spacing = 16.0f;
			std::vector<WidgetPtr> children;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) override;
		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseButtonUp(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseWheel(const Geometry& allocatedGeometry, const Vec2& mousePos, float scrollDelta) override;

		void addChild(WidgetPtr child);

	private:

		float m_spacing = 16.0f;
		std::vector<WidgetPtr> m_children;

	};

}
