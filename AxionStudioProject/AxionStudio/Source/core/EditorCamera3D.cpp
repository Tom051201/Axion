#include "axpch.h"
#include "EditorCamera3D.h"

#include "AxionEngine/Source/input/Input.h"

namespace Axion {

	EditorCamera3D::EditorCamera3D(float aspectRatio)
		: Camera(Mat4::perspective(Math::toRadians(m_fov), aspectRatio, 0.1f, 100.0f)), m_aspectRatio(aspectRatio) {

		recalculateViewMatrix();
	}

	EditorCamera3D::~EditorCamera3D() {}

	void EditorCamera3D::resize(float width, float height) {
		m_aspectRatio = width / height;
		m_projectionMatrix = Mat4::perspective(Math::toRadians(m_fov), m_aspectRatio, 0.1f, 100.0f);
		recalculateViewMatrix();
	}

	void EditorCamera3D::onUpdate(Timestep ts) {
		float velocity = m_translationSpeed * ts;

		if (Input::isKeyPressed(KeyCode::W)) m_position += m_forward * velocity;
		if (Input::isKeyPressed(KeyCode::S)) m_position -= m_forward * velocity;
		if (Input::isKeyPressed(KeyCode::A)) m_position -= m_right * velocity;
		if (Input::isKeyPressed(KeyCode::D)) m_position += m_right * velocity;
		if (Input::isKeyPressed(KeyCode::Q)) m_position -= m_up * velocity;
		if (Input::isKeyPressed(KeyCode::E)) m_position += m_up * velocity;

		recalculateViewMatrix();
		//AX_LOG_TRACE("Camera Pos: {} {} {}", m_position.x, m_position.y, m_position.z);
	}

	void EditorCamera3D::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<MouseScrolledEvent>(AX_BIND_EVENT_FN(EditorCamera3D::onMouseScrolled));
		dispatcher.dispatch<MouseMovedEvent>(AX_BIND_EVENT_FN(EditorCamera3D::onMouseMoved));
		dispatcher.dispatch<WindowResizeEvent>(AX_BIND_EVENT_FN(EditorCamera3D::onWindowResize));
	}

	void EditorCamera3D::recalculateViewMatrix() {
		Vec3 direction;
		direction.x = cos(Math::toRadians(m_yaw)) * cos(Math::toRadians(m_pitch));
		direction.y = sin(Math::toRadians(m_pitch));
		direction.z = sin(Math::toRadians(m_yaw)) * cos(Math::toRadians(m_pitch));
		m_forward = Vec3::normalize(direction);

		m_right = Vec3::normalize(Vec3::cross(m_forward, m_worldUp));
		m_up = Vec3::normalize(Vec3::cross(m_right, m_forward));

		m_viewMatrix = Mat4::lookAt(m_position, m_position + m_forward, m_up);
		m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
	}

	bool EditorCamera3D::onMouseScrolled(MouseScrolledEvent& e) {
		m_fov -= e.getYOffset();
		m_fov = std::clamp(m_fov, 20.0f, 90.0f);
		m_projectionMatrix = Mat4::perspective(Math::toRadians(m_fov), m_aspectRatio, 0.1f, 100.0f);
		recalculateViewMatrix();
		return false;
	}

	bool EditorCamera3D::onMouseMoved(MouseMovedEvent& e) {
		if (m_firstMouse) {
			m_lastMouseX = e.getX();
			m_lastMouseY = e.getY();
			m_firstMouse = false;
			return false;
		}

		float xOffset = e.getX() - m_lastMouseX;
		float yOffset = m_lastMouseY - e.getY();

		m_lastMouseX = e.getX();
		m_lastMouseY = e.getY();

		xOffset *= m_rotationSpeed;
		yOffset *= m_rotationSpeed;

		m_yaw += xOffset;
		m_pitch += yOffset;

		m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

		recalculateViewMatrix();
		return false;
	}

	bool EditorCamera3D::onWindowResize(WindowResizeEvent& e) {
		resize((float)e.getWidth(), (float)e.getHeight());
		return false;
	}

}
