#include "Sandbox2D.h"

// TEMP
#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Platform/directx/D12FrameBuffer.h"

Sandbox2D::Sandbox2D() : Layer("Sandbox2D"), m_cameraController(1280.0f / 720.0f, true) {}

void Sandbox2D::onAttach() {
	m_buffer1 = Axion::ConstantBuffer::create(sizeof(Axion::ObjectBuffer));

	m_buffer2 = Axion::ConstantBuffer::create(sizeof(Axion::ObjectBuffer));
	m_texture = Axion::Texture2D::create("Sandbox/Assets/logo.png");

	Axion::Renderer2D::setClearColor({ 0.4f, 0.1f, 0.2f, 1.0f });

	Axion::FrameBufferSpecification fbs;
	fbs.width = 1280.0f;
	fbs.height = 720.0f;
	m_frameBuffer = Axion::FrameBuffer::create(fbs);

	m_context = static_cast<Axion::D12Context*>(Axion::GraphicsContext::get()->getNativeContext());
}

void Sandbox2D::onDetach() {
	m_buffer1->release();
	m_buffer2->release();
	m_texture->release();
	m_frameBuffer->release();
}

void Sandbox2D::onUpdate(Axion::Timestep ts) {

	m_cameraController.onUpdate(ts);

	Axion::Renderer2D::beginScene(m_cameraController.getCamera());
	m_frameBuffer->bind();
	m_frameBuffer->clear({ 0.0f, 0.0f, 0.0f, 1.0f });

	Axion::Renderer2D::drawQuad({ -0.55f, 0.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, m_buffer1);

	Axion::Renderer2D::drawTexture({ 0.55f, 0.0f, 0.0f }, { 1.0f, 1.0f }, m_texture, m_buffer2);

	m_frameBuffer->unbind();

	m_context->getCommandListWrapper().close();
	m_context->getCommandQueueWrapper().executeCommandList(m_context->getCommandListWrapper().getCommandList());
	m_context->waitForPreviousFrame();
	m_context->getCommandListWrapper().reset();

}

void Sandbox2D::onEvent(Axion::Event& e) {
	m_cameraController.onEvent(e);
}

void Sandbox2D::onGuiRender() {
	Axion::D12FrameBuffer* framebuffer = static_cast<Axion::D12FrameBuffer*>(m_frameBuffer.get());
	Axion::D12Context* context = static_cast<Axion::D12Context*>(Axion::GraphicsContext::get()->getNativeContext());
	ImGui::Begin("Image");
	ImGui::Image(
		(ImTextureID)context->getSrvHeapWrapper().getGpuHandle(framebuffer->getSrvHeapIndex()).ptr,
		ImVec2((float)framebuffer->getSpecification().width, (float)framebuffer->getSpecification().height)
	);
	ImGui::End();
}
