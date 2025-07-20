#include "Sandbox2D.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

Sandbox2D::Sandbox2D() : Layer("Sandbox2D"), m_cameraController(1280.0f / 720.0f, true), m_camera(1280.0f / 720.0f) {}

void Sandbox2D::onAttach() {

	Axion::Renderer2D::setClearColor({ 0.3f, 0.3f, 0.3f, 1.0f });

	m_texture = Axion::Texture2D::create("Sandbox/Assets/logo.png");

	Axion::FrameBufferSpecification fbs;
	fbs.width = 1280;
	fbs.height = 720;
	m_frameBuffer = Axion::FrameBuffer::create(fbs);

	// sets up the material
	Axion::Ref<Axion::Shader> shader = Axion::Shader::create("materialShader");
	shader->compileFromFile("Sandbox/Assets/shaders/ColorShader.hlsl");
	m_material = Axion::Material::create("Material2D", Axion::Vec4(0.0f, 0.0f, 1.0f, 1.0f), shader);

	// creates a constant buffer for uploading to the shader
	m_uploadBuffer = Axion::ConstantBuffer::create(sizeof(Axion::ObjectBuffer));

}

void Sandbox2D::onDetach() {
	m_texture->release();
	m_frameBuffer->release();
}

void Sandbox2D::onUpdate(Axion::Timestep ts) {

	m_cameraController.onUpdate(ts);
	m_camera.onUpdate(ts);

	Axion::Renderer2D::beginScene(m_camera);
	m_frameBuffer->bind();
	m_frameBuffer->clear();

	Axion::Renderer2D::drawQuad(Axion::Mat4::TRS(Axion::Vec3::zero(), Axion::Vec3::zero(), Axion::Vec3::one()), m_material, m_uploadBuffer);

	Axion::Renderer2D::endScene();
	m_frameBuffer->unbind();

	Axion::Renderer::renderToSwapChain();
}

void Sandbox2D::onEvent(Axion::Event& e) {
	m_cameraController.onEvent(e);
	m_camera.onEvent(e);
}

void Sandbox2D::onGuiRender() {
	ImGui::Begin("Image");
	ImGui::Image(
		(ImTextureID)m_frameBuffer->getColorAttachmentHandle(),
		ImVec2((float)m_frameBuffer->getSpecification().width, (float)m_frameBuffer->getSpecification().height)
	);
	ImGui::End();
}
