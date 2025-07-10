#include "Sandbox2D.h"
#include "Sandbox3D.h"

Sandbox3D::Sandbox3D() : Layer("Sandbox3D"), m_camController(1280.0f / 720.0f, true) {}

Sandbox3D::~Sandbox3D() {}

void Sandbox3D::onAttach() {

}

void Sandbox3D::onDetach() {

}

void Sandbox3D::onUpdate(Axion::Timestep ts) {
	m_camController.onUpdate(ts);

	//Axion::Renderer2D::beginScene(m_camController.getCamera());
	//
	//Axion::Renderer2D::drawQuad({ -0.55f, 0.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, m_buffer);
	//
	//Axion::Renderer2D::endScene();
}

void Sandbox3D::onEvent(Axion::Event& e) {
	m_camController.onEvent(e);
}

void Sandbox3D::onGuiRender() {

}
