#pragma once

#include "AxionEngine/Source/core/Math.h"

namespace Axion {

	class WireframeRenderer {
	public:

		static void drawBox(const Mat4& transform, const Vec4& color);
		static void drawSphere(const Mat4& transform, float radius, const Vec4& color);
		static void drawCapsule(const Mat4& transform, float radius, float halfHeight, const Vec4& color);

	};

}
