#include "axpch.h"
#include "OrthographicCamera.h"

namespace Axion {

	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top, float nearZ, float farZ)
		: m_position(Vec3::zero()), m_rotationX(0.0f), m_rotationY(0.0f), m_rotationZ(0.0f) {

		m_projectionMatrix = Mat4::orthographic(right - left, top - bottom, nearZ, farZ);
		recalculateViewMatrix();

	}

	void OrthographicCamera::setProjection(float left, float right, float bottom, float top, float nearZ, float farZ) {
		m_projectionMatrix = Mat4::orthographic(right - left, top - bottom, nearZ, farZ);
		recalculateViewMatrix();
	}

	void OrthographicCamera::setPosition(const Vec3& pos) {
		m_position = pos;
		recalculateViewMatrix();
	}

	void OrthographicCamera::setRotationX(float rot) {
		m_rotationX = rot;
		recalculateViewMatrix();
	}

	void OrthographicCamera::setRotationY(float rot) {
		m_rotationY = rot;
		recalculateViewMatrix();
	}

	void OrthographicCamera::setRotationZ(float rot) {
		m_rotationZ = rot;
		recalculateViewMatrix();
	}

	void OrthographicCamera::recalculateViewMatrix() {
		Mat4 transform = Mat4::rotationZ(m_rotationZ) * Mat4::translation(m_position);
		m_viewMatrix = transform.inverse();
		m_viewProjectionMatrix = m_viewMatrix * m_projectionMatrix;
	}

}
