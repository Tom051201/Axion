#pragma once

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/core/Timestep.h"
#include "AxionEngine/Source/events/MouseEvent.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"
#include "AxionEngine/Source/render/Camera.h"

namespace Axion {

	class EditorCamera : public Camera {
	public:

		EditorCamera(float aspectRatio);
		~EditorCamera();

		void resize(float width, float height);

		void onUpdate(Timestep ts);
		void onEvent(Event& e);

		void setPosition(const Vec3& pos);
		const Vec3& getPosition() const { return m_position; }

		void setRotation(const Vec3& rot);
		const Vec3& getRotation() const { return m_rotation; }

	private:

		float m_aspectRatio;
		float m_zoomLevel = 1.0f;

		Vec3 m_position = Vec3::zero();
		Vec3 m_rotation = Vec3::zero();
		float m_translationSpeed = 1.0f;
		float m_rotationSpeed = 1.0f;

		bool onMouseScrolled(MouseScrolledEvent& e);
		bool onWindowResize(WindowResizeEvent& e);

		void recalculateViewMatrix();

		void setProjection(float left, float right, float bottom, float top, float nearZ = -1.0f, float farZ = 1.0f);
	};

}