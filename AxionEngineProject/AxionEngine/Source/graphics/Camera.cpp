#include "axpch.h"
#include "Camera.h"

namespace Axion {

	void Camera::setPerspective(float verticalFOV, float nearClip, float farClip) {
		m_projectionType = ProjectionType::Perspective;
		m_perspectiveFOV = verticalFOV;
		m_perspectiveNear = nearClip;
		m_perspectiveFar = farClip;
		recalculateProjection();
	}

	void Camera::setOrthographic(float size, float nearClip, float farClip) {
		m_projectionType = ProjectionType::Orthographic;
		m_orthographicSize = size;
		m_orthographicNear = nearClip;
		m_orthographicFar = farClip;
		recalculateProjection();
	}

	void Camera::setViewportSize(uint32_t width, uint32_t height) {
		if (height == 0) return;

		m_aspectRatio = (float)width / (float)height;
		recalculateProjection();
	}

	void Camera::setViewMatrix(const Mat4& viewMatrix) {
		m_viewMatrix = viewMatrix;
		m_viewProjectionMatrix = m_viewMatrix * m_projectionMatrix;
	}

	// -- Perspective --
	void Camera::setPerspectiveVerticalFOV(float fov) {
		m_perspectiveFOV = fov;
		recalculateProjection();
	}

	void Camera::setPerspectiveNearClip(float nearClip) {
		m_perspectiveNear = nearClip;
		recalculateProjection();
	}

	void Camera::setPerspectiveFarClip(float farClip) {
		m_perspectiveFar = farClip;
		recalculateProjection();
	}

	// -- Orthographic --
	void Camera::setOrthographicSize(float size) {
		m_orthographicSize = size;
		recalculateProjection();
	}

	void Camera::setOrthographicNearClip(float nearClip) {
		m_orthographicNear = nearClip;
		recalculateProjection();
	}

	void Camera::setOrthographicFarClip(float farClip) {
		m_orthographicFar = farClip;
		recalculateProjection();
	}

	void Camera::recalculateProjection() {
		if (m_projectionType == ProjectionType::Perspective) {
			// -- Perspective --
			m_projectionMatrix = Mat4::perspective(m_perspectiveFOV, m_aspectRatio, m_perspectiveNear, m_perspectiveFar);
		}
		else {
			// -- Orthographic --
			float orthoLeft = -m_orthographicSize * m_aspectRatio * 0.5f;
			float orthoRight = m_orthographicSize * m_aspectRatio * 0.5f;
			float orthoBottom = -m_orthographicSize * 0.5f;
			float orthoTop = m_orthographicSize * 0.5f;

			m_projectionMatrix = Mat4::orthographicOffCenter(orthoLeft, orthoRight, orthoBottom, orthoTop, m_orthographicNear, m_orthographicFar);

		}
	}

}
