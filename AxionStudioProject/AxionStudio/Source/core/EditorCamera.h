#pragma once

#include "AxionEngine/Source/render/Camera.h"

#include "AxionEngine/Source/core/Timestep.h"
#include "AxionEngine/Source/events/Event.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"
#include "AxionEngine/Source/events/MouseEvent.h"

namespace Axion {

	class EditorCameraPanel;

	class EditorCamera : public Camera {
	public:

		EditorCamera(uint32_t width, uint32_t height);
		~EditorCamera() = default;

		void onUpdate(Timestep ts);
		void onEvent(Event& e);
		void resize(uint32_t width, uint32_t height);

		void set2D();
		void set3D();
		bool is2D();
		bool is3D();

		void setPosition(const Vec3& pos);
		const Vec3& getPosition() const { return m_position; }
		Vec3& getPosition() { return m_position; }

		void setHoveringSceneViewport(bool hovering) { m_hoveringSceneViewport = hovering; }

	private:

		Vec3 m_position = { 0.0f, 0.0f, -3.0f };
		bool m_hoveringSceneViewport = false;
		Vec2 m_initialMousePosition = { 0.0f, 0.0f };
		Vec2 m_lastMousePosition = { 0.0f, 0.0f };
		bool m_isActive = false;
		float m_viewportWidth = 1280.0f;
		float m_viewportHeight = 720.0f;

		// -- 3D --
		float m_pitch = 0.0f;
		float m_yaw = 0.0f;
		float m_distance = 10.0f;
		float m_fov = 45.0f;
		float m_translationSpeed3D = 5.0f;
		float m_rotationSpeed3D = 0.003f;

		// -- 2D --
		float m_zoom2D = 10.0f;
		float m_minZoom2D = 0.1f;
		float m_maxZoom2D = 10.0f;
		float m_keyboardSpeed2D = 5.0f;
		float m_dragSpeed2D = 0.01f;

		void updateView();
		void updateCamera3D(Timestep ts);
		void updateCamera2D(Timestep ts);

		bool onMouseScrolled(MouseScrolledEvent& e);
		bool onMouseMoved(MouseMovedEvent& e);
		bool onMouseButtonPressed(MouseButtonPressedEvent& e);
		bool onMouseButtonReleased(MouseButtonReleasedEvent& e);

		Vec3 getForwardDirection() const;
		Vec3 getRightDirection() const;
		Vec3 getUpDirection() const;

		friend class EditorCameraPanel;
	};

}
