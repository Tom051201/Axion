#include "Sandbox2D.h"

#include <imgui.h>

#define PROFILE_SCOPE(name) Axion::ScopedTimer timer##__LINE__(name, [&](ProfileResult profileResult) { m_profileResults.push_back(profileResult); });

Sandbox2D::Sandbox2D() : Layer("Sandbox2D"), m_cameraController(1280.0f / 720.0f, true) {}

void Sandbox2D::onAttach() {}

void Sandbox2D::onDetach() {}

void Sandbox2D::onUpdate(Axion::Timestep ts) {

	PROFILE_SCOPE("Sandbox2D::onUpdate");

	{
		PROFILE_SCOPE("CameraController::onUpdate");
		m_cameraController.onUpdate(ts);
	}

	{
		PROFILE_SCOPE("Rendering");
		Axion::Renderer2D::beginScene(m_cameraController.getCamera());

		Axion::Renderer2D::setClearColor({ 0.4f, 0.1f, 0.2f, 1.0f });
		Axion::Renderer2D::clear();

		Axion::Renderer2D::drawQuad({ 0.3f, 0.0f, 0.0f }, { 1.5f, 0.7f }, { 0.0f, 1.0f, 0.0f, 1.0f });
	}
}

void Sandbox2D::onGuiRender() {
	ImGui::Begin("Settings");
	
	for (auto& result : m_profileResults) {
		char label[50];
		strcpy(label, "%.3fms ");
		strcat(label, result.name);
		ImGui::Text(label, result.time);
	}
	m_profileResults.clear();

	ImGui::End();
}

void Sandbox2D::onEvent(Axion::Event& e) {
	m_cameraController.onEvent(e);
}
