#include "Sandbox3D.h"

#include "AxionEngine/Source/scene/SceneManager.h"

Sandbox3D::Sandbox3D() : Layer("Sandbox3D"), m_camera(1280.0f / 720.0f) {}

Sandbox3D::~Sandbox3D() {}

void Sandbox3D::onAttach() {
	Axion::SceneManager::newScene();
	m_scene = Axion::SceneManager::getScene();


	Axion::AssetHandle<Axion::Skybox> skyboxHandle = Axion::AssetManager::load<Axion::Skybox>(std::filesystem::absolute("Sandbox/Assets/skybox/BlueSkybox.axsky").string());
	m_scene->setSkybox(skyboxHandle);
}

void Sandbox3D::onDetach() {

}

void Sandbox3D::onUpdate(Axion::Timestep ts) {
	m_camera.onUpdate(ts);
	
	Axion::Renderer3D::beginScene(m_camera);

	m_scene->onUpdate(ts, m_camera);

	Axion::Renderer3D::endScene();
}

void Sandbox3D::onEvent(Axion::Event& e) {
	m_camera.onEvent(e);
}

void Sandbox3D::onGuiRender() {}
