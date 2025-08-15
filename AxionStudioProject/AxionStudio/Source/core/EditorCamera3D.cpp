#include "axpch.h"
#include "EditorCamera3D.h"

#include "AxionEngine/Source/core/Application.h"
#include "AxionEngine/Source/input/Input.h"

namespace Axion {

	EditorCamera3D::EditorCamera3D(float aspectRatio)
		: m_aspectRatio(aspectRatio) {

		m_nearClip = 0.1f;
		m_farClip = 100.0f;
		m_fov = 45.0f;

		setProjection(m_fov, m_aspectRatio, m_nearClip, m_farClip);
		recalculateViewMatrix();
	}

	EditorCamera3D::~EditorCamera3D() {}

	void EditorCamera3D::resize(float width, float height) {
		m_aspectRatio = width / height;
		setProjection(m_fov, m_aspectRatio, m_nearClip, m_farClip);
	}

	void EditorCamera3D::onUpdate(Timestep ts) {
		Vec3 forward = getForward();
		Vec3 right = getRight();
		Vec3 up = getUp();

		float speed = m_translationSpeed;
		if (Input::isKeyPressed(KeyCode::LeftShift)) {
			speed *= 3.0f;
		}

		if (Input::isKeyPressed(KeyCode::W)) m_position += forward * speed * ts;
		if (Input::isKeyPressed(KeyCode::S)) m_position -= forward * speed * ts;
		if (Input::isKeyPressed(KeyCode::D)) m_position -= right * speed * ts;
		if (Input::isKeyPressed(KeyCode::A)) m_position += right * speed * ts;
		if (Input::isKeyPressed(KeyCode::E)) m_position += up * speed * ts;
		if (Input::isKeyPressed(KeyCode::Q)) m_position -= up * speed * ts;

		recalculateViewMatrix();
	}

	void EditorCamera3D::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<MouseScrolledEvent>(AX_BIND_EVENT_FN(EditorCamera3D::onMouseScrolled));
		dispatcher.dispatch<MouseMovedEvent>(AX_BIND_EVENT_FN(EditorCamera3D::onMouseMoved));
		dispatcher.dispatch<MouseButtonPressedEvent>(AX_BIND_EVENT_FN(EditorCamera3D::onMouseButtonPressed));
		dispatcher.dispatch<MouseButtonReleasedEvent>(AX_BIND_EVENT_FN(EditorCamera3D::onMouseButtonReleased));
		dispatcher.dispatch<WindowResizeEvent>(AX_BIND_EVENT_FN(EditorCamera3D::onWindowResize));
	}

	bool EditorCamera3D::onMouseScrolled(MouseScrolledEvent& e) {
		m_fov -= e.getYOffset();
		m_fov = Math::clamp(m_fov, 1.0f, 90.0f);
		setProjection(m_fov, m_aspectRatio, m_nearClip, m_farClip);
		return false;
	}

	bool EditorCamera3D::onMouseMoved(MouseMovedEvent& e) {
		if (!m_lookModeActive) return false;

		auto& window = Application::get().getWindow();
		int winCenterX = window.getWidth() / 2;
		int winCenterY = window.getHeight() / 2;

		float deltaX = e.getX() - winCenterX;
		float deltaY = winCenterY - e.getY();

		if (deltaX != 0.0f || deltaY != 0.0f) {
			m_yaw += deltaX * m_mouseSensitivity;
			m_pitch += deltaY * m_mouseSensitivity;
			m_pitch = Math::clamp(m_pitch, -DirectX::XM_PIDIV2 + 0.01f, DirectX::XM_PIDIV2 - 0.01f);

			auto& cursor = Application::get().getCursor();
			cursor.centerInWindow();
		}

		return false;
	}

	bool EditorCamera3D::onMouseButtonPressed(MouseButtonPressedEvent& e) {
		if (e.getMouseButton() == MouseButton::Right && !m_lookModeActive) {
			m_savedCursorPosition = Input::getMousePosition();
			auto& cursor = Application::get().getCursor();
			cursor.hide();
			cursor.centerInWindow();
			m_lookModeActive = true;
		}
		return false;
	}

	bool EditorCamera3D::onMouseButtonReleased(MouseButtonReleasedEvent& e) {
		if (e.getMouseButton() == MouseButton::Right && m_lookModeActive) {
			auto& cursor = Application::get().getCursor();
			cursor.show();
			cursor.setPositionInWindow((uint32_t)m_savedCursorPosition.x, (uint32_t)m_savedCursorPosition.y);
			m_lookModeActive = false;
		}
		return false;
	}

	bool EditorCamera3D::onWindowResize(WindowResizeEvent& e) {
		resize((float)e.getWidth(), (float)e.getHeight());

		if (m_lookModeActive) {
			auto& cursor = Application::get().getCursor();
			cursor.centerInWindow();
		}
		return false;
	}

	void EditorCamera3D::setPosition(const Vec3& pos) {
		m_position = pos;
		recalculateViewMatrix();
	}

	void EditorCamera3D::recalculateViewMatrix() {
		Vec3 forward = getForward();
		Vec3 target = m_position + forward;
		m_viewMatrix = Mat4::lookAt(m_position, target, Vec3::up());
		m_viewProjectionMatrix = m_viewMatrix * m_projectionMatrix;
	}

	void EditorCamera3D::setProjection(float fovDegrees, float aspect, float nearZ, float farZ) {
		m_projectionMatrix = Axion::Mat4::perspective(Axion::Math::toRadians(fovDegrees), aspect, nearZ, farZ);
		recalculateViewMatrix();
	}

	Axion::Vec3 EditorCamera3D::getForward() const {
		float yawRad = m_yaw;
		float pitchRad = m_pitch;

		Vec3 forward;
		forward.x = cosf(pitchRad) * sinf(yawRad);
		forward.y = sinf(pitchRad);
		forward.z = cosf(pitchRad) * cosf(yawRad);
		return forward.normalized();
	}

	Axion::Vec3 EditorCamera3D::getRight() const {
		return getForward().cross(Vec3::up()).normalized();
	}

	Axion::Vec3 EditorCamera3D::getUp() const {
		return getRight().cross(getForward()).normalized();
	}

}
