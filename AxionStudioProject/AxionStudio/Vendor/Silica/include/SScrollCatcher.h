#pragma once

#include "SWidget.h"

namespace Silica {

	class SScrollCatcher : public SWidget {
	public:

		struct Args {
			std::function<EventReply(float)> onMouseWheel;
			WidgetPtr child;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& geo) override;
		void onDraw(DrawList& drawList, const Geometry& geo) const override;

		EventReply onMouseMove(const Geometry& geom, const Vec2& pos) override;
		EventReply onMouseButtonDown(const Geometry& geom, const Vec2& pos, MouseButton btn) override;
		EventReply onMouseButtonUp(const Geometry& geom, const Vec2& pos, MouseButton btn) override;
		EventReply onMouseWheel(const Geometry& geom, const Vec2& pos, float delta) override;

	private:

		std::function<EventReply(float)> m_onMouseWheel;
		WidgetPtr m_child;

	};

}
