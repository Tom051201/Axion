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

		void setPosition(const Vec3& pos);
		const Vec3& getPosition() const { return m_position; }
		Vec3& getPosition() { return m_position; }

		void setTranslationSpeed(float speed) { m_translationSpeed = speed; }
		float getTranslationSpeed() { return m_translationSpeed; }
		float* getTranslationSpeedData() { return &m_translationSpeed; }

		void setFov(float fov) { m_fov = fov; }
		float getFov() { return m_fov; }
		float* getFovData() { return &m_fov; }

	private:

		float m_aspectRatio;

		Vec3 m_position = Vec3(0.0f, 0.0f, -3.0f);
		float m_translationSpeed = 1.0f;
		float m_mouseSensitivity = 0.003f;

		float m_nearClip;
		float m_farClip;
		float m_fov;
		float m_yaw = 0.0f;
		float m_pitch = 0.0f;

		bool m_lookModeActive = false;
		Vec2 m_savedCursorPosition{ 0.0f, 0.0f };

		bool onMouseScrolled(MouseScrolledEvent& e);
		bool onMouseMoved(MouseMovedEvent& e);
		bool onMouseButtonPressed(MouseButtonPressedEvent& e);
		bool onMouseButtonReleased(MouseButtonReleasedEvent& e);
		bool onWindowResize(WindowResizeEvent& e);

		void recalculateViewMatrix();
		void setProjection(float fovDegrees, float aspect, float nearZ, float farZ);

		Vec3 getForward() const;
		Vec3 getRight() const;
		Vec3 getUp() const;

	};

}
