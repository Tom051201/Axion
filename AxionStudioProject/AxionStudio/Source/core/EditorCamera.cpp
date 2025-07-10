#include "EditorCamera.h"

#include "AxionEngine/Source/input/Input.h"

namespace Axion {

	EditorCamera::EditorCamera(float aspectRatio)
		: Camera(Mat4::orthographic(-m_aspectRatio * m_zoomLevel, m_aspectRatio* m_zoomLevel, -m_zoomLevel, m_zoomLevel)),
		m_aspectRatio(aspectRatio) {

		recalculateViewMatrix();
	}

	EditorCamera::~EditorCamera() {}

	void EditorCamera::resize(float width, float height) {
		m_aspectRatio = width / height;
		setProjection(-m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel, -m_zoomLevel, m_zoomLevel);
	}

	void EditorCamera::onUpdate(Timestep ts) {

		if (Input::isKeyPressed(KeyCode::A)) m_position.x -= m_translationSpeed * ts;
		if (Input::isKeyPressed(KeyCode::D)) m_position.x += m_translationSpeed * ts;
		if (Input::isKeyPressed(KeyCode::W)) m_position.y += m_translationSpeed * ts;
		if (Input::isKeyPressed(KeyCode::S)) m_position.y -= m_translationSpeed * ts;

		if (Input::isKeyPressed(KeyCode::Q)) m_rotation.z += m_rotationSpeed * ts;
		if (Input::isKeyPressed(KeyCode::E)) m_rotation.z -= m_rotationSpeed * ts;

		setRotation(m_rotation);
		setPosition(m_position);
	}

	void EditorCamera::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<MouseScrolledEvent>(AX_BIND_EVENT_FN(EditorCamera::onMouseScrolled));
		dispatcher.dispatch<WindowResizeEvent>(AX_BIND_EVENT_FN(EditorCamera::onWindowResize));
	}

	bool EditorCamera::onMouseScrolled(MouseScrolledEvent& e) {
		m_zoomLevel -= e.getYOffset() * 0.2f;
		m_zoomLevel = std::max(m_zoomLevel, 0.25f);
		setProjection(-m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel, -m_zoomLevel, m_zoomLevel);
		return false;
	}

	bool EditorCamera::onWindowResize(WindowResizeEvent& e) {
		resize((float)e.getWidth(), (float)e.getHeight());
		return false;
	}

	void EditorCamera::setPosition(const Vec3& pos) {
		m_position = pos;
		recalculateViewMatrix();
	}

	void EditorCamera::setRotation(const Vec3& rot) {
		m_rotation = rot;
		recalculateViewMatrix();
	}

	void EditorCamera::recalculateViewMatrix() {
		Mat4 transform = Mat4::translation(m_position) * Mat4::rotationZ(m_rotation.z);
		m_viewMatrix = transform.inverse();
		m_viewProjectionMatrix = m_viewMatrix * m_projectionMatrix;
	}

	void EditorCamera::setProjection(float width, float height, float nearZ, float farZ) {
		m_projectionMatrix = Mat4::orthographic(width, height, nearZ, farZ);
		recalculateViewMatrix();
	}

}