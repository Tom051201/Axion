#pragma once

#include "SWidget.h"
#include "MathTypes.h"
#include "Geometry.h"

namespace Silica {

	class SSpacer : public SWidget {
	public:

		struct Args {
			Vec2 size = Vec2::zero();
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

	private:

		Vec2 m_size;

	};

}
