#include "Sandbox2D.h"

Sandbox2D::Sandbox2D() : Layer("Sandbox2D"), m_cameraController(1280.0f / 720.0f, true) {}

void Sandbox2D::onAttach() {

	//m_cameraCB = Axion::ConstantBuffer::create(sizeof(Axion::SceneBuffer));

	m_shader = Axion::Shader::create("quadShader2D");
	m_shader->compileFromFile("assets/Shader1.hlsl");

	std::vector<Axion::Vertex> vertices = {
		Axion::Vertex(-0.5f, -0.5f, 0.0f,	1.0f, 1.0f, 0.0f, 1.0f,		0.0f, 0.0f),
		Axion::Vertex( 0.5f, -0.5f, 0.0f,	0.0f, 0.0f, 1.0f, 1.0f,		1.0f, 0.0f),
		Axion::Vertex( 0.5f,  0.5f, 0.0f,	0.0f, 1.0f, 0.0f, 1.0f,		1.0f, 1.0f),
		Axion::Vertex(-0.5f,  0.5f, 0.0f,	1.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f)
	};
	std::vector<uint32_t> indices = { 0, 2, 1,	2, 0, 3 };
	m_quad = Axion::Mesh::create(vertices, indices);
	m_quadCB = Axion::ConstantBuffer::create(sizeof(Axion::ObjectBuffer));
	m_quadTransform = Axion::Mat4::identity();
}

void Sandbox2D::onDetach() {
	m_cameraCB->release();
	m_shader->release();
	m_quad->release();
	m_quadCB->release();
}

void Sandbox2D::onUpdate(Axion::Timestep ts) {

	m_cameraController.onUpdate(ts);

	//Axion::Renderer2D::beginScene(m_cameraController.getCamera());
	//Axion::Renderer::clear(0.0f, 0.1f, 0.2f, 1.0f);

	// camera
	//Axion::SceneBuffer sceneBuffer;
	//sceneBuffer.viewProjection = DirectX::XMMatrixTranspose(m_cameraController.getCamera().getViewProjectionMatrix().toXM());
	//m_cameraCB->update(&sceneBuffer, sizeof(Axion::SceneBuffer));

	//Axion::Renderer2D::drawQuad({ 0.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });

	//m_shader->bind();
	//Axion::ObjectBuffer quadData;
	//quadData.modelMatrix = DirectX::XMMatrixTranspose(m_quadTransform.toXM());
	//m_quadCB->update(&quadData, sizeof(Axion::ObjectBuffer));
	//Axion::Renderer::submit(m_quad, m_cameraCB, 0, m_quadCB, 1);
}

void Sandbox2D::onEvent(Axion::Event& e) {
	m_cameraController.onEvent(e);
}
