#pragma once

#include "Axion/core/Math.h"

namespace Axion {

	class Camera {
	public:

		Camera(const Mat4& projection) : m_projectionMatrix(projection) {}

		const Mat4& getProjection() const { return m_projectionMatrix; }

	private:

		Mat4 m_projectionMatrix;

	};

}
