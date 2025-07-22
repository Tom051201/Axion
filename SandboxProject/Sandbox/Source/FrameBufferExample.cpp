#include "FrameBufferExample.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

FrameBufferExample::FrameBufferExample() : Layer("FrameBufferExample"), m_cameraController(1280.0f / 720.0f, true) {}

void FrameBufferExample::onAttach() {

	Axion::Renderer2D::setClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });

	// creates framebuffer
	Axion::FrameBufferSpecification fbs;
	fbs.width = 1280;
	fbs.height = 720;
	fbs.textureFormat = Axion::ColorFormat::RGBA8;
	fbs.depthStencilFormat = Axion::DepthStencilFormat::DEPTH32F;
	m_frameBuffer = Axion::FrameBuffer::create(fbs);

	// creates shader and materials
	Axion::ShaderSpecification shaderSpec;
	shaderSpec.name = "materialShader";
	Axion::Ref<Axion::Shader> shader = Axion::Shader::create(shaderSpec);
	shader->compileFromFile("Sandbox/Assets/shaders/ColorShader.hlsl");

	m_material = Axion::Material::create("Material2D", Axion::Vec4(0.0f, 0.0f, 1.0f, 1.0f), shader);
	m_material2 = Axion::Material::create("Material", Axion::Vec4(1.0f, 0.0f, 0.0f, 1.0f), shader);

	// creates constant buffers for uploading to the shader
	m_uploadBuffer = Axion::ConstantBuffer::create(sizeof(Axion::ObjectBuffer));
	m_uploadBuffer2 = Axion::ConstantBuffer::create(sizeof(Axion::ObjectBuffer));
}

void FrameBufferExample::onDetach() {
	m_frameBuffer->release();

	m_material->release();
	m_material2->release();

	m_uploadBuffer->release();
	m_uploadBuffer2->release();
}

void FrameBufferExample::onUpdate(Axion::Timestep ts) {
	m_cameraController.onUpdate(ts);

	Axion::Renderer2D::beginScene(m_cameraController.getCamera());
	m_frameBuffer->bind();
	m_frameBuffer->clear();

	Axion::Renderer2D::drawQuad(Axion::Mat4::TRS(Axion::Vec3::zero(), Axion::Vec3::zero(), Axion::Vec3::one()), m_material, m_uploadBuffer);
	Axion::Renderer2D::drawQuad(Axion::Mat4::TRS({ -0.3f, 0.3f, 0.3f }, Axion::Vec3::zero(), Axion::Vec3::one()), m_material2, m_uploadBuffer2);

	Axion::Renderer2D::endScene();
	m_frameBuffer->unbind();

	Axion::Renderer::renderToSwapChain();
}

void FrameBufferExample::onEvent(Axion::Event& e) {
	m_cameraController.onEvent(e);
}

void FrameBufferExample::onGuiRender() {
	ImGui::Begin("Image");
	ImGui::Image(
		(ImTextureID)m_frameBuffer->getColorAttachmentHandle(),
		ImVec2((float)m_frameBuffer->getSpecification().width, (float)m_frameBuffer->getSpecification().height)
	);
	ImGui::End();
}

