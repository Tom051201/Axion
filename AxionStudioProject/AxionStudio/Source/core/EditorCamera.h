#pragma once

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/core/Timestep.h"
#include "AxionEngine/Source/events/MouseEvent.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"
#include "AxionEngine/Source/render/Camera.h"

namespace Axion {

	class EditorCamera : public Camera {
	public:

		EditorCamera(float aspectRatio);
		~EditorCamera();

		void resize(float width, float height);

		void onUpdate(Timestep ts);
		void onEvent(Event& e);

		const Mat4& getViewProjectionMatrix() const { return m_viewProjectionMatrix; }

	private:

		float m_aspectRatio;
		float m_zoomLevel = 1.0f;

		Mat4 m_viewMatrix;
		Mat4 m_viewProjectionMatrix;
		Vec3 m_position = { 0.0f, 0.0f, 0.0f };
		Vec3 m_rotation = { 0.0f, 0.0f, 0.0f };
		float m_translationSpeed = 1.0f;
		float m_rotationSpeed = 1.0f;

		bool onMouseScrolled(MouseScrolledEvent& e);
		bool onWindowResize(WindowResizeEvent& e);

		void setPosition(const Vec3& pos);
		void setRotation(const Vec3& rot);
		void recalculateViewMatrix();

		void setProjection(const Mat4& projection) override;
		void setProjection(float width, float height, float nearZ = -1.0f, float farZ = 1.0f);
	};

}
