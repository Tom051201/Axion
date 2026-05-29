#pragma once

#include "MathTypes.h"

namespace Silica {

	struct Geometry {
		Vec2 position;
		Vec2 size;

		bool contains(const Vec2& point) const {
			return point.x >= position.x && point.x <= position.x + size.x &&
				point.y >= position.y && point.y <= position.y + size.y;
		}
	};

}
