#include "axpch.h"
#include "EditorCamera2D.h"

#include "AxionEngine/Source/input/Input.h"
#include "AxionEngine/Source/core/Application.h"

namespace Axion {

	EditorCamera2D::EditorCamera2D(float width, float height)
		: m_width(width), m_height(height) {

		recalculateProjection();
		recalculateViewMatrix();
	}

	void EditorCamera2D::resize(float width, float height) {
		m_width = width;
		m_height = height;
		recalculateProjection();
	}

	void EditorCamera2D::onUpdate(Timestep ts) {
		if (!m_hoveringSceneViewport) return;

		float speed = m_keyboardSpeed / std::max(m_zoom, 0.0001f);

		if (Input::isKeyPressed(KeyCode::W)) m_position.y += speed * ts;
		if (Input::isKeyPressed(KeyCode::S)) m_position.y -= speed * ts;
		if (Input::isKeyPressed(KeyCode::D)) m_position.x += speed * ts;
		if (Input::isKeyPressed(KeyCode::A)) m_position.x -= speed * ts;

		recalculateViewMatrix();
		recalculateProjection();
	}

	void EditorCamera2D::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<MouseScrolledEvent>(AX_BIND_EVENT_FN(EditorCamera2D::onMouseScrolled));
		dispatcher.dispatch<MouseMovedEvent>(AX_BIND_EVENT_FN(EditorCamera2D::onMouseMoved));
		dispatcher.dispatch<MouseButtonPressedEvent>(AX_BIND_EVENT_FN(EditorCamera2D::onMouseButtonPressed));
		dispatcher.dispatch<MouseButtonReleasedEvent>(AX_BIND_EVENT_FN(EditorCamera2D::onMouseButtonReleased));
		dispatcher.dispatch<WindowResizeEvent>(AX_BIND_EVENT_FN(EditorCamera2D::onWindowResize));
	}

	void EditorCamera2D::setPosition(const Vec2& pos) {
		m_position = pos;
		recalculateViewMatrix();
	}

	void EditorCamera2D::setZoom(float zoom) {
		m_zoom = Math::clamp(zoom, m_minZoom, m_maxZoom);
		recalculateViewMatrix();
		recalculateProjection();
	}

	bool EditorCamera2D::onMouseScrolled(MouseScrolledEvent& e) {
		if (!m_hoveringSceneViewport) return false;

		float delta = e.getYOffset();
		setZoom(m_zoom - delta * 0.1f);

		return false;
	}

	bool EditorCamera2D::onMouseMoved(MouseMovedEvent& e) {
		if (!m_dragging || !m_hoveringSceneViewport) return false;

		auto& window = Application::get().getWindow();
		float centerX = window.getWidth() * 0.5f;
		float centerY = window.getHeight() * 0.5f;

		float deltaX = e.getX() - centerX;
		float deltaY = centerY - e.getY();

		if (deltaX != 0.0f || deltaY != 0.0f) {
			float zoomFactor = std::log2(m_zoom + 1.0f);
			float moveScale = 1.0f / std::max(zoomFactor, 0.1f);

			m_position.x -= deltaX * moveScale * m_dragSpeed;
			m_position.y -= deltaY * moveScale * m_dragSpeed;

			auto& cursor = Application::get().getCursor();
			cursor.centerInWindow();
		}

		recalculateViewMatrix();
		recalculateProjection();

		return false;
	}

	bool EditorCamera2D::onMouseButtonPressed(MouseButtonPressedEvent& e) {
		if (e.getMouseButton() == MouseButton::Right && m_hoveringSceneViewport && !m_dragging) {
			m_dragging = true;

			auto pos = Input::getMousePosition();
			m_savedCursorPosition = { pos.first, pos.second };

			auto& cursor = Application::get().getCursor();
			cursor.hide();
			cursor.centerInWindow();
		}

		return false;
	}

	bool EditorCamera2D::onMouseButtonReleased(MouseButtonReleasedEvent& e) {
		if (e.getMouseButton() == MouseButton::Right && m_dragging) {
			m_dragging = false;

			auto& cursor = Application::get().getCursor();
			cursor.show();
			cursor.setPositionInWindow((uint32_t)m_savedCursorPosition.x, (uint32_t)m_savedCursorPosition.y);
		}

		return false;
	}

	bool EditorCamera2D::onWindowResize(WindowResizeEvent& e) {
		resize((float)e.getWidth(), (float)e.getHeight());

		if (m_dragging) {
			auto& cursor = Application::get().getCursor();
			cursor.centerInWindow();
		}

		return false;
	}

	void EditorCamera2D::recalculateViewMatrix() {
		Mat4 transform = Mat4::translation(Vec3(m_position.x, m_position.y, 0.0f));
		m_viewMatrix = transform.inverse();
		m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
	}

	void EditorCamera2D::recalculateProjection() {
		float aspect = m_width / m_height;
		float size = m_zoom;

		m_projectionMatrix = Mat4::orthographicOffCenter(
			-aspect * size, +aspect * size,
			-size, +size,
			-1.0f, 1.0f
		);

		m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
	}

}
