#pragma once

#include "AxionEngine/Source/render/OrthographicCamera.h"
#include "AxionEngine/Source/core/Timestep.h"
#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"
#include "AxionEngine/Source/events/MouseEvent.h"

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
