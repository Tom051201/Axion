#pragma once

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/core/Timestep.h"
#include "AxionEngine/Source/events/MouseEvent.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"
#include "AxionEngine/Source/render/Camera.h"

class SandboxCamera : public Axion::Camera {
public:

	SandboxCamera(float aspectRatio);
	~SandboxCamera();

	void resize(float width, float height);

	void onUpdate(Axion::Timestep ts);
	void onEvent(Axion::Event& e);

	void setPosition(const Axion::Vec3& pos);
	const Axion::Vec3& getPosition() const { return m_position; }

	void setRotation(const Axion::Vec3& rot);
	const Axion::Vec3& getRotation() const { return m_rotation; }

private:

	float m_aspectRatio;
	float m_zoomLevel = 1.0f;

	Axion::Vec3 m_position = Axion::Vec3(0.0f, 0.0f, -3.0f);
	Axion::Vec3 m_rotation = Axion::Vec3::zero();
	float m_translationSpeed = 1.0f;
	float m_rotationSpeed = 1.0f;

	float m_nearClip;
	float m_farClip;
	float m_fov;
	float m_yaw = 0.0f;
	float m_pitch = 0.0f;

	bool onMouseScrolled(Axion::MouseScrolledEvent& e);
	bool onMouseMoved(Axion::MouseMovedEvent& e);
	bool onWindowResize(Axion::WindowResizeEvent& e);

	void recalculateViewMatrix();
	void setProjection(float left, float right, float bottom, float top, float nearZ = -1.0f, float farZ = 1.0f);
	void setProjectionPerspective(float fovDegrees, float aspect, float nearZ, float farZ);

	Axion::Vec3 getForward() const;
	Axion::Vec3 getRight() const;
	Axion::Vec3 getUp() const;

};
