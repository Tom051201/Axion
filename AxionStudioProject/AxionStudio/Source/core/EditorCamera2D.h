#pragma once

#include "AxionEngine/Source/core/Timestep.h"
#include "AxionEngine/Source/render/Camera.h"
#include "AxionEngine/Source/events/Event.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"
#include "AxionEngine/Source/events/MouseEvent.h"

namespace Axion {

	class EditorCamera2D : public Camera {
	public:

		EditorCamera2D(float width, float height);
		~EditorCamera2D() = default;

		void resize(float width, float height);

		void onUpdate(Timestep ts);
		void onEvent(Event& e);

		void setPosition(const Vec2& pos);
		const Vec2& getPosition() const { return m_position; }
		Vec2& getPosition() { return m_position; }

		void setZoom(float zoom);
		float getZoom() const { return m_zoom; }

		void setIsHoveringSceneViewport(bool hovering) { m_hoveringSceneViewport = hovering; }
		bool isHoveringSceneViewport() const { return m_hoveringSceneViewport; }

	private:

		Vec2 m_position = { 0.0f, 0.0f };
		float m_width = 1.0f;
		float m_height = 1.0f;

		float m_zoom = 1.0f;
		float m_minZoom = 0.1f;
		float m_maxZoom = 10.0f;

		bool m_dragging = false;
		Vec2 m_lastMousePosition;

		bool m_hoveringSceneViewport = false;

		bool onMouseScrolled(MouseScrolledEvent& e);
		bool onMouseMoved(MouseMovedEvent& e);
		bool onMouseButtonPressed(MouseButtonPressedEvent& e);
		bool onMouseButtonReleased(MouseButtonReleasedEvent& e);
		bool onWindowResize(WindowResizeEvent& e);

		void recalculateViewMatrix();
		void recalculateProjection();

	};

}
