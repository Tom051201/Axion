#include "Sandbox2D.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

Sandbox2D::Sandbox2D() : Layer("Sandbox2D"), m_cameraController(1280.0f / 720.0f, true) {}

void Sandbox2D::onAttach() {

	Axion::Renderer2D::setClearColor({ 0.3f, 0.3f, 0.3f, 1.0f });

	// sets up the material
	Axion::BufferLayout vertexLayout = {
		{ "POSITION", Axion::ShaderDataType::Float3 },
		{ "COLOR", Axion::ShaderDataType::Float4 },
		{ "TEXCOORD", Axion::ShaderDataType::Float2 }
	};
	Axion::ShaderSpecification shaderSpec;
	shaderSpec.name = "materialShader";
	shaderSpec.vertexLayout = vertexLayout;
	Axion::Ref<Axion::Shader> shader = Axion::Shader::create(shaderSpec);
	shader->compileFromFile("Sandbox/Assets/shaders/ColorShader.hlsl");
	m_material = Axion::Material::create("Material2D", Axion::Vec4(0.0f, 0.0f, 1.0f, 1.0f), shader);
	m_material2 = Axion::Material::create("Material", Axion::Vec4(1.0f, 0.0f, 0.0f, 1.0f), shader);

	// creates a constant buffer for uploading to the shader
	m_uploadBuffer = Axion::ConstantBuffer::create(sizeof(Axion::ObjectBuffer));
	m_uploadBuffer2 = Axion::ConstantBuffer::create(sizeof(Axion::ObjectBuffer));

}

void Sandbox2D::onDetach() {
	m_material->release();
	m_material2->release();
	m_uploadBuffer->release();
	m_uploadBuffer2->release();
}

void Sandbox2D::onUpdate(Axion::Timestep ts) {
	m_cameraController.onUpdate(ts);

	Axion::Renderer2D::beginScene(m_cameraController.getCamera());

	Axion::Renderer2D::drawQuad(Axion::Mat4::TRS(Axion::Vec3::zero(), Axion::Vec3::zero(), Axion::Vec3::one()), m_material, m_uploadBuffer);
	Axion::Renderer2D::drawQuad(Axion::Mat4::TRS({-0.3f, 0.3f, 0.3f}, Axion::Vec3::zero(), Axion::Vec3::one()), m_material2, m_uploadBuffer2);

	Axion::Renderer2D::endScene();
}

void Sandbox2D::onEvent(Axion::Event& e) {
	m_cameraController.onEvent(e);
}

void Sandbox2D::onGuiRender() {}
