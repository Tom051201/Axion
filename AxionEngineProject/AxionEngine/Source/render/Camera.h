#pragma once

#include "AxionEngine/Source/core/Math.h"

namespace Axion {

	class Camera {
	public:

		Camera() = default;
		Camera(const Mat4& projection) : m_projectionMatrix(projection) {}
		virtual ~Camera() = default;

		const Mat4& getProjectionMatrix() const { return m_projectionMatrix; }
		const Mat4& getViewMatrix() const { return m_viewMatrix; }
		const Mat4& getViewProjectionMatrix() const { return m_viewProjectionMatrix; }

	protected:

		Mat4 m_projectionMatrix;
		Mat4 m_viewMatrix;
		Mat4 m_viewProjectionMatrix;

	};

}
