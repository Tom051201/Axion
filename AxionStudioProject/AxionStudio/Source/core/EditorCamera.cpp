#include "EditorCamera.h"

#include "AxionEngine/Source/input/Input.h"
#include "AxionEngine/Source/core/Application.h"

namespace Axion {

	EditorCamera::EditorCamera(uint32_t width, uint32_t height)
		: Camera((float)width / (float)height), m_viewportWidth((float)width), m_viewportHeight((float)height) {

		set3D();
	}

	void EditorCamera::onUpdate(Timestep ts) {
		if (m_hoveringSceneViewport) {
			if (is3D()) {
				updateCamera3D(ts);
			}
			else {
				updateCamera2D(ts);
			}
		}

		updateView();
	}

	void EditorCamera::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<MouseScrolledEvent>(AX_BIND_EVENT_FN(EditorCamera::onMouseScrolled));
		dispatcher.dispatch<MouseMovedEvent>(AX_BIND_EVENT_FN(EditorCamera::onMouseMoved));
		dispatcher.dispatch<MouseButtonPressedEvent>(AX_BIND_EVENT_FN(EditorCamera::onMouseButtonPressed));
		dispatcher.dispatch<MouseButtonReleasedEvent>(AX_BIND_EVENT_FN(EditorCamera::onMouseButtonReleased));
	}

	void EditorCamera::resize(uint32_t width, uint32_t height) {
		m_viewportWidth = (float)width;
		m_viewportHeight = (float)height;
		setViewportSize((uint32_t)width, (uint32_t)height);

		if (is3D()) {
			setPerspective(Math::toRadians(m_fov), 0.1f, 100.0f);
		}
		else {
			setOrthographic(m_zoom2D, -100.0f, 100.0f);
		}
	}

	void EditorCamera::set2D() {
		m_projectionType = ProjectionType::Orthographic;
		m_pitch = 0.0f;
		m_yaw = 0.0f;
		m_position.z = 10.0f;

		setOrthographic(m_zoom2D, -100.0f, 100.0f);
		updateView();
	}

	void EditorCamera::set3D() {
		m_projectionType = ProjectionType::Perspective;

		setPerspective(Math::toRadians(45.0f), 0.1f, 100.0f);
		updateView();
	}

	bool EditorCamera::is2D() {
		return m_projectionType == ProjectionType::Orthographic;
	}

	bool EditorCamera::is3D() {
		return m_projectionType == ProjectionType::Perspective;
	}

	void EditorCamera::setPosition(const Vec3& pos) {
		m_position = pos;
		updateView();
	}

	void EditorCamera::updateView() {
		if (is3D()) {
			Vec3 forward = getForwardDirection();
			Vec3 target = m_position + forward;
			setViewMatrix(Mat4::lookAt(m_position, target, Vec3::up()));
		}
		else {
			Mat4 transform = Mat4::translation(m_position);
			setViewMatrix(transform.inverse());
		}
	}

	void EditorCamera::updateCamera3D(Timestep ts) {
		if (Input::isMouseButtonPressed(MouseButton::Right)) {
			Vec3 forward = getForwardDirection();
			Vec3 right = getRightDirection();
			Vec3 up = getUpDirection();

			float speed = m_translationSpeed3D;
			if (Input::isKeyPressed(KeyCode::LeftShift)) speed *= 3.0f;

			if (Input::isKeyPressed(KeyCode::W)) m_position += forward * speed * ts;
			if (Input::isKeyPressed(KeyCode::S)) m_position -= forward * speed * ts;
			if (Input::isKeyPressed(KeyCode::D)) m_position -= right * speed * ts;
			if (Input::isKeyPressed(KeyCode::A)) m_position += right * speed * ts;
			if (Input::isKeyPressed(KeyCode::E)) m_position += up * speed * ts;
			if (Input::isKeyPressed(KeyCode::Q)) m_position -= up * speed * ts;
		}
	}

	void EditorCamera::updateCamera2D(Timestep ts) {
		if (!Input::isMouseButtonPressed(MouseButton::Right)) {
			float speed = m_keyboardSpeed2D / std::max(m_zoom2D, 0.0001f);
			if (Input::isKeyPressed(KeyCode::W)) m_position.y += speed * ts;
			if (Input::isKeyPressed(KeyCode::S)) m_position.y -= speed * ts;
			if (Input::isKeyPressed(KeyCode::D)) m_position.x += speed * ts;
			if (Input::isKeyPressed(KeyCode::A)) m_position.x -= speed * ts;
		}
	}

	bool EditorCamera::onMouseScrolled(MouseScrolledEvent& e) {
		if (!m_hoveringSceneViewport) return false;

		float delta = e.getYOffset();

		if (is3D()) {
			m_fov -= delta;
			m_fov = Math::clamp(m_fov, 1.0f, 90.0f);
			setPerspective(Math::toRadians(m_fov), 0.1f, 100.0f);
		}
		else {
			m_zoom2D -= delta * 0.1f;
			m_zoom2D = Math::clamp(m_zoom2D, m_minZoom2D, m_maxZoom2D);
			setOrthographic(m_zoom2D, -100.0f, 100.0f);

		}

		updateView();
		return false;
	}

	bool EditorCamera::onMouseMoved(MouseMovedEvent& e) {
		if (!m_isActive) return false;

		auto& window = Application::get().getWindow();
		float centerX = window.getWidth() * 0.5f;
		float centerY = window.getHeight() * 0.5f;

		float deltaX = e.getX() - centerX;
		float deltaY = centerY - e.getY();
		if (deltaX == 0.0f || deltaY == 0.0f) return false;

		if (is3D()) {
			m_yaw += deltaX * m_rotationSpeed3D;
			m_pitch += deltaY * m_rotationSpeed3D;
			m_pitch = std::clamp(m_pitch, -1.56f, 1.56f);

			Application::get().getCursor().centerInWindow();
		}
		else {
			float zoomFactor = std::log2(m_zoom2D + 1.0f);
			float moveScale = 1.0f / std::max(zoomFactor, 0.1f);

			m_position.x -= deltaX * moveScale * m_dragSpeed2D;
			m_position.y -= deltaY * moveScale * m_dragSpeed2D;

			Application::get().getCursor().centerInWindow();
		}

		return false;
	}

	bool EditorCamera::onMouseButtonPressed(MouseButtonPressedEvent& e) {
		if (e.getMouseButton() == MouseButton::Right && m_hoveringSceneViewport) {
			m_isActive = true;
			m_initialMousePosition = Input::getMousePosition();

			Application::get().getCursor().hide();
			Application::get().getCursor().centerInWindow();
		}
		return false;
	}

	bool EditorCamera::onMouseButtonReleased(MouseButtonReleasedEvent& e) {
		if (e.getMouseButton() == MouseButton::Right) {
			m_isActive = false;

			Application::get().getCursor().show();
			Application::get().getCursor().setPositionInWindow((uint32_t)m_initialMousePosition.x, (uint32_t)m_initialMousePosition.y);
		}
		return false;
	}

	Vec3 EditorCamera::getForwardDirection() const {
		float yawRad = m_yaw;
		float pitchRad = m_pitch;

		Vec3 forward;
		forward.x = cosf(pitchRad) * sinf(yawRad);
		forward.y = sinf(pitchRad);
		forward.z = cosf(pitchRad) * cosf(yawRad);
		return forward.normalized();
	}

	Vec3 EditorCamera::getRightDirection() const {
		return getForwardDirection().cross(Vec3::up()).normalized();
	}

	Vec3 EditorCamera::getUpDirection() const {
		return getRightDirection().cross(getForwardDirection()).normalized();
	}

}
