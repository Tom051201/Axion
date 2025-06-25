#pragma once

#include "Axion/render/OrthographicCamera.h"
#include "Axion/core/Timestep.h"
#include "Axion/core/Math.h"
#include "Axion/events/ApplicationEvent.h"
#include "Axion/events/MouseEvent.h"

namespace Axion {

	class OrthographicCameraController {
	public:

		OrthographicCameraController(float aspectRatio, bool rotation = false);

		void resize(float width, float height);

		void onUpdate(Timestep ts);
		void onEvent(Event& e);

		OrthographicCamera& getCamera() { return m_camera; }
		const OrthographicCamera& getCamera() const { return m_camera; }

	private:

		float m_aspectRatio;
		float m_zoomLevel = 1.0f;
		OrthographicCamera m_camera;

		bool m_rotationOn;
		Vec3 m_position = { 0.0f, 0.0f, 0.0f };
		float m_rotation = 0.0f;
		float m_translationSpeed = 1.0f;
		float m_rotationSpeed = 1.0f;

		bool onMouseScrolled(MouseScrolledEvent& e);
		bool onWindowResized(WindowResizeEvent& e);

	};

}
