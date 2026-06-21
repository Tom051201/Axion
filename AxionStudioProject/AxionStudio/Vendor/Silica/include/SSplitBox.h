#pragma once

#include "SWidget.h"

namespace Silica {

	class SSplitBox : public SWidget {
	public:

		struct Args {
			float leftWidth = 250.0f;
			WidgetPtr leftContent;
			WidgetPtr rightContent;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		EventReply onMouseMove(const Geometry& geo, const Vec2& pos) override;
		EventReply onMouseButtonDown(const Geometry& geo, const Vec2& pos, MouseButton btn) override;
		EventReply onMouseButtonUp(const Geometry& geo, const Vec2& pos, MouseButton btn) override;
		EventReply onMouseWheel(const Geometry& geo, const Vec2& pos, float delta) override;

	private:

		float m_leftWidth = 250.0f;
		WidgetPtr m_left;
		WidgetPtr m_right;

	};

}
