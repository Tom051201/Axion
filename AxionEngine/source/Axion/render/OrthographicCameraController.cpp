#include "axpch.h"
#include "OrthographicCameraController.h"

#include "Axion/input/Input.h"
#include "Axion/input/InputCodes.h"

namespace Axion {

	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotationOn)
		: m_aspectRatio(aspectRatio),
		  m_camera(-m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel, -m_zoomLevel, m_zoomLevel), m_rotationOn(rotationOn) {
	}

	void OrthographicCameraController::resize(float width, float height) {
		m_aspectRatio = width / height;
		m_camera.setProjection(-m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel, -m_zoomLevel, m_zoomLevel);
	}

	void OrthographicCameraController::onUpdate(Timestep ts) {

		if (Input::isKeyPressed(KeyCode::A)) m_position.x -= m_translationSpeed * ts;
		if (Input::isKeyPressed(KeyCode::D)) m_position.x += m_translationSpeed * ts;
		if (Input::isKeyPressed(KeyCode::W)) m_position.y += m_translationSpeed * ts;
		if (Input::isKeyPressed(KeyCode::S)) m_position.y -= m_translationSpeed * ts;

		if (m_rotationOn) {
			if (Input::isKeyPressed(KeyCode::Q)) m_rotation += m_rotationSpeed * ts;
			if (Input::isKeyPressed(KeyCode::E)) m_rotation -= m_rotationSpeed * ts;
			m_camera.setRotationZ(m_rotation);
		}

		m_camera.setPosition(m_position);
	}

	void OrthographicCameraController::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<MouseScrolledEvent>(AX_BIND_EVENT_FN(OrthographicCameraController::onMouseScrolled));
		dispatcher.dispatch<WindowResizeEvent>(AX_BIND_EVENT_FN(OrthographicCameraController::onWindowResized));
	}

	bool OrthographicCameraController::onMouseScrolled(MouseScrolledEvent& e) {

		m_zoomLevel -= e.getYOffset() * 0.2f;
		m_zoomLevel = std::max(m_zoomLevel, 0.25f);
		m_camera.setProjection(-m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel, -m_zoomLevel, m_zoomLevel);

		return false;
	}

	bool OrthographicCameraController::onWindowResized(WindowResizeEvent& e) {

		resize((float)e.getWidth(), (float)e.getHeight());

		return false;
	}

}
