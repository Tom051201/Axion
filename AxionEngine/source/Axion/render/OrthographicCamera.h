#pragma once
#include "axpch.h"

#include "Axion/core/Math.h"

namespace Axion {

	class OrthographicCamera {
	public:

		OrthographicCamera(float left, float right, float bottom, float top, float nearZ = -1.0f, float farZ = 1.0f);

		void setProjection(float left, float right, float bottom, float top, float nearZ = -1.0f, float farZ = 1.0f);

		void setPosition(const Vec3& pos);
		void setRotationZ(float rot);

		const Vec3& getPosition() const { return m_position; };
		float getRotationZ() const { return m_rotationZ; }


		const Mat4& getProjectionMatrix() const { return m_projectionMatrix; }
		const Mat4& getViewMatrix() const { return m_viewMatrix; }
		const Mat4& getViewProjectionMatrix() const { return m_viewProjectionMatrix; }

	private:

		Mat4 m_projectionMatrix;
		Mat4 m_viewMatrix;
		Mat4 m_viewProjectionMatrix;

		Vec3 m_position;
		float m_rotationZ = 0;

		void recalculateViewMatrix();
	};


}

