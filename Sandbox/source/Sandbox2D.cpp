#include "Sandbox2D.h"

Sandbox2D::Sandbox2D() : Layer("Sandbox2D"), m_cameraController(1280.0f / 720.0f, true) {}

void Sandbox2D::onAttach() {
}

void Sandbox2D::onDetach() {
}

void Sandbox2D::onUpdate(Axion::Timestep ts) {

	m_cameraController.onUpdate(ts);

	Axion::Renderer2D::beginScene(m_cameraController.getCamera());

	Axion::Renderer2D::setClearColor({ 0.4f, 0.1f, 0.2f, 1.0f });
	Axion::Renderer2D::clear();

	Axion::Renderer2D::drawQuad({ 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });

}

void Sandbox2D::onEvent(Axion::Event& e) {
	m_cameraController.onEvent(e);
}
