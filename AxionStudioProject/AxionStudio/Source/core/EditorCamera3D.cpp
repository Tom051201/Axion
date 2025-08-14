#include "axpch.h"
#include "EditorCamera3D.h"

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

		if (Input::isKeyPressed(KeyCode::W)) m_position += forward * m_translationSpeed * ts;
		if (Input::isKeyPressed(KeyCode::S)) m_position -= forward * m_translationSpeed * ts;
		if (Input::isKeyPressed(KeyCode::D)) m_position -= right * m_translationSpeed * ts;
		if (Input::isKeyPressed(KeyCode::A)) m_position += right * m_translationSpeed * ts;
		if (Input::isKeyPressed(KeyCode::E)) m_rotation -= up * m_rotationSpeed * ts;
		if (Input::isKeyPressed(KeyCode::Q)) m_rotation += up * m_rotationSpeed * ts;

		if (Input::isKeyPressed(KeyCode::R)) {
			if (Input::isKeyPressed(KeyCode::LeftShift)) {
				m_rotation.x -= m_rotationSpeed * ts;
			}
			else {
				m_rotation.x += m_rotationSpeed * ts;
			}
		}
		if (Input::isKeyPressed(KeyCode::F)) {
			if (Input::isKeyPressed(KeyCode::LeftShift)) {
				m_rotation.y -= m_rotationSpeed * ts;
			}
			else {
				m_rotation.y += m_rotationSpeed * ts;
			}
		}

		recalculateViewMatrix();
	}

	void EditorCamera3D::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<MouseScrolledEvent>(AX_BIND_EVENT_FN(EditorCamera3D::onMouseScrolled));
		dispatcher.dispatch<MouseMovedEvent>(AX_BIND_EVENT_FN(EditorCamera3D::onMouseMoved));
		dispatcher.dispatch<WindowResizeEvent>(AX_BIND_EVENT_FN(EditorCamera3D::onWindowResize));
	}

	bool EditorCamera3D::onMouseScrolled(MouseScrolledEvent& e) {
		m_zoomLevel -= e.getYOffset() * 0.2f;
		m_zoomLevel = std::max(m_zoomLevel, 0.25f);
		m_fov += m_zoomLevel;
		return false;
	}

	bool EditorCamera3D::onMouseMoved(MouseMovedEvent& e) {
		static bool firstMouse = true;
		static float lastX = 0.0f;
		static float lastY = 0.0f;

		float x = e.getX();
		float y = e.getY();

		if (firstMouse) {
			lastX = x;
			lastY = y;
			firstMouse = false;
			return false;
		}

		float deltaX = x - lastX;
		float deltaY = lastY - y;

		lastX = x;
		lastY = y;

		float sensitivity = 0.003f;
		m_yaw += deltaX * sensitivity;
		m_pitch += deltaY * sensitivity;

		m_pitch = Math::clamp(m_pitch, -DirectX::XM_PIDIV2 + 0.01f, DirectX::XM_PIDIV2 - 0.01f);
		return false;
	}

	bool EditorCamera3D::onWindowResize(WindowResizeEvent& e) {
		resize((float)e.getWidth(), (float)e.getHeight());
		return false;
	}

	void EditorCamera3D::setPosition(const Vec3& pos) {
		m_position = pos;
		recalculateViewMatrix();
	}

	void EditorCamera3D::setRotation(const Vec3& rot) {
		m_rotation = rot;
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
