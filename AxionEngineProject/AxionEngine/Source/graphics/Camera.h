#pragma once

#include "AxionEngine/Source/core/Math.h"

namespace Axion {

	class Camera {
	public:

		enum class ProjectionType {
			Perspective = 0,
			Orthographic = 1
		};

		Camera() = default;
		Camera(float aspectRatio) : m_aspectRatio(aspectRatio) {}
		virtual ~Camera() = default;

		virtual void setPerspective(float verticalFOV, float nearClip, float farClip);
		virtual void setOrthographic(float size, float nearClip, float farClip);

		virtual void setViewportSize(uint32_t width, uint32_t height);

		virtual ProjectionType getProjectionType() const { return m_projectionType; }
		virtual void setProjectionType(ProjectionType type) { m_projectionType = type; }

		virtual float getAspectRatio() const { return m_aspectRatio; }

		virtual void setViewMatrix(const Mat4& viewMatrix);
		virtual const Mat4& getViewMatrix() const { return m_viewMatrix; }
		virtual const Mat4& getProjectionMatrix() const { return m_projectionMatrix; }
		virtual const Mat4& getViewProjectionMatrix() const { return m_viewProjectionMatrix; }

		// -- Perspective --
		virtual float getPerspectiveVerticalFOV() const { return m_perspectiveFOV; }
		virtual float getPerspectiveVerticalFOV() { return m_perspectiveFOV; }
		virtual void setPerspectiveVerticalFOV(float fov);

		virtual float getPerspectiveNearClip() const { return m_perspectiveNear; }
		virtual float getPerspectiveNearClip() { return m_perspectiveNear; }
		virtual void setPerspectiveNearClip(float nearClip);

		virtual float getPerspectiveFarClip() const { return m_perspectiveFar; }
		virtual float getPerspectiveFarClip() { return m_perspectiveFar; }
		virtual void setPerspectiveFarClip(float farClip);

		// -- Orthographic --
		virtual float getOrthographicSize() const { return m_orthographicSize; }
		virtual float getOrthographicSize() { return m_orthographicSize; }
		virtual void setOrthographicSize(float size);

		virtual float getOrthographicNearClip() const { return m_orthographicNear; }
		virtual float getOrthographicNearClip() { return m_orthographicNear; }
		virtual void setOrthographicNearClip(float nearClip);

		virtual float getOrthographicFarClip() const { return m_orthographicFar; }
		virtual float getOrthographicFarClip() { return m_orthographicFar; }
		virtual void setOrthographicFarClip(float farClip);

	protected:

		ProjectionType m_projectionType = ProjectionType::Perspective;
		float m_aspectRatio = 16.0f / 9.0f;
		Mat4 m_viewMatrix = Mat4::identity();
		Mat4 m_projectionMatrix = Mat4::identity();
		Mat4 m_viewProjectionMatrix = Mat4::identity();

		// -- Perspective --
		float m_perspectiveFOV = Math::toRadians(45.0f);
		float m_perspectiveNear = 0.1f;
		float m_perspectiveFar = 100.0f;

		// -- Orthographic --
		float m_orthographicSize = 10.0f;
		float m_orthographicNear = -1.0f;
		float m_orthographicFar = 1.0f;

		virtual void recalculateProjection();

	};

}
