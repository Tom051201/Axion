#pragma once

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/core/Timestep.h"
#include "AxionEngine/Source/events/MouseEvent.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"
#include "AxionEngine/Source/render/Camera.h"

namespace Axion {

	class EditorCamera3D : public Camera {
	public:

		EditorCamera3D(float aspectRatio);
		~EditorCamera3D();

		void resize(float width, float height);

		void onUpdate(Timestep ts);
		void onEvent(Event& e);

		const Vec3& getPosition() const { return m_position; }

	private:

		Vec3 m_position = { 0.0f, 0.0f, 3.0f };
		Vec3 m_forward = { 0.0f, 0.0f, -1.0f };
		Vec3 m_up = { 0.0f, 1.0f, 0.0f };
		Vec3 m_right = { 1.0f, 0.0f, 0.0f };
		Vec3 m_worldUp = { 0.0f, 1.0f, 0.0f };

		float m_yaw = -90.0f;
		float m_pitch = 0.0f;

		float m_fov = 45.0f;
		float m_aspectRatio;

		float m_translationSpeed = 1.0f;
		float m_rotationSpeed = 1.0f;

		float m_lastMouseX = 0.0f;
		float m_lastMouseY = 0.0f;
		bool m_firstMouse = true;

		bool onMouseScrolled(MouseScrolledEvent& e);
		bool onMouseMoved(MouseMovedEvent& e);
		bool onWindowResize(WindowResizeEvent& e);

		void recalculateViewMatrix();

	};

}
