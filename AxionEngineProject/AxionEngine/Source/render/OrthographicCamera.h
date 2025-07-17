#pragma once
#include "axpch.h"

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/render/Camera.h"

namespace Axion {

	class OrthographicCamera : public Camera {
	public:

		OrthographicCamera(float left, float right, float bottom, float top, float nearZ = -1.0f, float farZ = 1.0f);

		void setProjection(float left, float right, float bottom, float top, float nearZ = -1.0f, float farZ = 1.0f);

		void setPosition(const Vec3& pos);
		void setRotationX(float rot);
		void setRotationY(float rot);
		void setRotationZ(float rot);

		const Vec3& getPosition() const { return m_position; };
		float getRotationZ() const { return m_rotationZ; }

	private:

		Vec3 m_position;
		float m_rotationX = 0;
		float m_rotationY = 0;
		float m_rotationZ = 0;

		void recalculateViewMatrix();
	};


}

