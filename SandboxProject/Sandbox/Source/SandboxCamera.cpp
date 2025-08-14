#include "SandboxCamera.h"

#include "AxionEngine/Source/input/Input.h"

SandboxCamera::SandboxCamera(float aspectRatio)
	: m_aspectRatio(aspectRatio) {

	m_nearClip = 0.1f;
	m_farClip = 100.0f;
	m_fov = 45.0f;

	setProjectionPerspective(m_fov, m_aspectRatio, m_nearClip, m_farClip);
//	setProjection(-m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel, -m_zoomLevel, m_zoomLevel);
	recalculateViewMatrix();
}

SandboxCamera::~SandboxCamera() {}

void SandboxCamera::resize(float width, float height) {
	m_aspectRatio = width / height;
//	setProjection(-m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel, -m_zoomLevel, m_zoomLevel);
	setProjectionPerspective(m_fov, m_aspectRatio, m_nearClip, m_farClip);
}

void SandboxCamera::onUpdate(Axion::Timestep ts) {

	Axion::Vec3 forward = getForward();
	Axion::Vec3 right = getRight();
	Axion::Vec3 up = getUp();

	if (Axion::Input::isKeyPressed(Axion::KeyCode::W)) m_position += forward * m_translationSpeed * ts;
	if (Axion::Input::isKeyPressed(Axion::KeyCode::S)) m_position -= forward * m_translationSpeed * ts;
	if (Axion::Input::isKeyPressed(Axion::KeyCode::D)) m_position -= right * m_translationSpeed * ts;
	if (Axion::Input::isKeyPressed(Axion::KeyCode::A)) m_position += right * m_translationSpeed * ts;
	if (Axion::Input::isKeyPressed(Axion::KeyCode::E)) m_rotation -= up * m_rotationSpeed * ts;
	if (Axion::Input::isKeyPressed(Axion::KeyCode::Q)) m_rotation += up * m_rotationSpeed * ts;

	if (Axion::Input::isKeyPressed(Axion::KeyCode::R)) {
		if (Axion::Input::isKeyPressed(Axion::KeyCode::LeftShift)) {
			m_rotation.x -= m_rotationSpeed * ts;
		}
		else {
			m_rotation.x += m_rotationSpeed * ts;
		}
	}
	if (Axion::Input::isKeyPressed(Axion::KeyCode::F)) {
		if (Axion::Input::isKeyPressed(Axion::KeyCode::LeftShift)) {
			m_rotation.y -= m_rotationSpeed * ts;
		}
		else {
			m_rotation.y += m_rotationSpeed * ts;
		}
	}

	recalculateViewMatrix();
}

void SandboxCamera::onEvent(Axion::Event& e) {
	Axion::EventDispatcher dispatcher(e);
	dispatcher.dispatch<Axion::MouseScrolledEvent>(AX_BIND_EVENT_FN(SandboxCamera::onMouseScrolled));
	dispatcher.dispatch<Axion::MouseMovedEvent>(AX_BIND_EVENT_FN(SandboxCamera::onMouseMoved));
	dispatcher.dispatch<Axion::WindowResizeEvent>(AX_BIND_EVENT_FN(SandboxCamera::onWindowResize));
}

bool SandboxCamera::onMouseScrolled(Axion::MouseScrolledEvent& e) {
	m_zoomLevel -= e.getYOffset() * 0.2f;
	m_zoomLevel = std::max(m_zoomLevel, 0.25f);
	m_fov += m_zoomLevel;
	return false;
}

bool SandboxCamera::onMouseMoved(Axion::MouseMovedEvent& e) {
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

	m_pitch = Axion::Math::clamp(m_pitch, -DirectX::XM_PIDIV2 + 0.01f, DirectX::XM_PIDIV2 - 0.01f);

	AX_LOG_TRACE(m_yaw);

	return false;
}

bool SandboxCamera::onWindowResize(Axion::WindowResizeEvent& e) {
	resize((float)e.getWidth(), (float)e.getHeight());
	return false;
}

void SandboxCamera::setPosition(const Axion::Vec3& pos) {
	m_position = pos;
	recalculateViewMatrix();
}

void SandboxCamera::setRotation(const Axion::Vec3& rot) {
	m_rotation = rot;
	recalculateViewMatrix();
}

void SandboxCamera::recalculateViewMatrix() {
	//Axion::Mat4 transform = Axion::Mat4::rotation(m_rotation) * Axion::Mat4::translation(m_position);
	////Axion::Mat4 transform = Axion::Mat4::rotationZ(m_rotation.z) * Axion::Mat4::translation(m_position);
	//m_viewMatrix = transform.inverse();
	//m_viewProjectionMatrix = m_viewMatrix * m_projectionMatrix;

	Axion::Vec3 forward = getForward();
	Axion::Vec3 target = m_position + forward;
	m_viewMatrix = Axion::Mat4::lookAt(m_position, target, Axion::Vec3::up());
	m_viewProjectionMatrix = m_viewMatrix * m_projectionMatrix;
}

void SandboxCamera::setProjection(float left, float right, float bottom, float top, float nearZ, float farZ) {
	m_projectionMatrix = Axion::Mat4::orthographic(right - left, top - bottom, nearZ, farZ);
	recalculateViewMatrix();
}

void SandboxCamera::setProjectionPerspective(float fovDegrees, float aspect, float nearZ, float farZ) {
	m_projectionMatrix = Axion::Mat4::perspective(Axion::Math::toRadians(fovDegrees) , aspect, nearZ, farZ);
	recalculateViewMatrix();
}

Axion::Vec3 SandboxCamera::getForward() const {
	float yawRad = m_yaw;
	float pitchRad = m_pitch;

	Axion::Vec3 forward;
	forward.x = cosf(pitchRad) * sinf(yawRad);
	forward.y = sinf(pitchRad);
	forward.z = cosf(pitchRad) * cosf(yawRad);
	return forward.normalized();
}

Axion::Vec3 SandboxCamera::getRight() const {
	return getForward().cross(Axion::Vec3::up()).normalized();
}

Axion::Vec3 SandboxCamera::getUp() const {
	return getRight().cross(getForward()).normalized();
}
