#pragma once

#include "AxionEngine/Source/core/Math.h"

namespace Axion {

	class Camera {
	public:

		Camera(const Mat4& projection) : m_projectionMatrix(projection) {}

		virtual void setProjection(const Mat4& projection) { m_projectionMatrix = projection; }
		virtual void setProjection(float width, float height, float nearZ = -1.0f, float farZ = 1.0f) { m_projectionMatrix = Mat4::orthographic(width, height, nearZ, farZ); }
		const Mat4& getProjection() const { return m_projectionMatrix; }

	protected:

		Mat4 m_projectionMatrix;

	};

}
