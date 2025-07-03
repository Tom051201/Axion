#include "Axion.h"
#include "AxionEngine/Source/core/EntryPoint.h"
#include "Sandbox2D.h"

class ExampleLayer : public Axion::Layer {
public:

	ExampleLayer() : Layer("Example"), m_cameraController(1280.0f / 720.0f, true) {

		m_shaderLibrary.load("quad", "Sandbox/Assets/Shader1.hlsl");

		m_quadMesh = Axion::Mesh::create(m_quadVertices, m_quadIndices);
		m_quadCB = Axion::ConstantBuffer::create(sizeof(Axion::ObjectBuffer));
		m_quadTransform = Axion::Mat4::identity();
		m_quadColor = { 1.0f, 0.0f, 0.0f, 1.0f };

		m_quadMesh2 = Axion::Mesh::create(m_quadVertices2, m_quadIndices2);
		m_quadCB2 = Axion::ConstantBuffer::create(sizeof(Axion::ObjectBuffer));
		m_quadTransform2 = Axion::Mat4::identity();
		m_quadTransform2.translation({ 1.0f, 1.0f, 1.0f });
		m_quadColor2 = { 1.0f, 0.0f, 0.0f, 1.0f };

		Axion::Renderer::setClearColor({ 0.0f, 0.1f, 0.2f, 1.0f });
	}

	void onDetach() override {

		m_shaderLibrary.release();

		m_quadMesh->release();
		m_quadCB->release();

		m_quadMesh2->release();
		m_quadCB2->release();
	}

	void onUpdate(Axion::Timestep ts) override {

		m_cameraController.onUpdate(ts);

		Axion::Renderer::beginScene(m_cameraController.getCamera());

		// quad
		Axion::ObjectBuffer quadData;
		quadData.modelMatrix = DirectX::XMMatrixTranspose(m_quadTransform.toXM());
		quadData.color = m_quadColor.toFloat4();
		m_quadCB->update(&quadData, sizeof(Axion::ObjectBuffer));
		Axion::Renderer::submit(m_quadMesh, m_quadCB, m_shaderLibrary.get("quad"));
	
	
		// quad
		Axion::ObjectBuffer quadData2;
		quadData2.modelMatrix = DirectX::XMMatrixTranspose(m_quadTransform2.toXM());
		quadData2.color = m_quadColor2.toFloat4();
		m_quadCB->update(&quadData2, sizeof(Axion::ObjectBuffer));
		Axion::Renderer::submit(m_quadMesh2, m_quadCB2, m_shaderLibrary.get("quad"));
	}

	void onEvent(Axion::Event& e) override {
		m_cameraController.onEvent(e);
	}

private:

	Axion::ShaderLibrary m_shaderLibrary;
	Axion::OrthographicCameraController m_cameraController;

	// quad
	std::vector<Axion::Vertex> m_quadVertices = {
		Axion::Vertex(-0.5f, -0.5f, 0.0f,	1.0f, 1.0f, 0.0f, 1.0f,		0.0f, 0.0f),
		Axion::Vertex( 0.5f, -0.5f, 0.0f,	0.0f, 0.0f, 1.0f, 1.0f,		1.0f, 0.0f),
		Axion::Vertex( 0.5f,  0.5f, 0.0f,	0.0f, 1.0f, 0.0f, 1.0f,		1.0f, 1.0f),
		Axion::Vertex(-0.5f,  0.5f, 0.0f,	1.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f)
	};
	std::vector<uint32_t> m_quadIndices = { 0, 2, 1,	2, 0, 3 };
	Axion::Ref<Axion::Mesh> m_quadMesh;
	Axion::Ref<Axion::ConstantBuffer> m_quadCB;
	Axion::Mat4 m_quadTransform;
	Axion::Vec4 m_quadColor;
	
	// quad 2
	std::vector<Axion::Vertex> m_quadVertices2 = {
		Axion::Vertex(-0.5f, -0.5f, 0.0f,	1.0f, 1.0f, 0.0f, 1.0f,		0.0f, 0.0f),
		Axion::Vertex(0.5f, -0.5f, 0.0f,	0.0f, 0.0f, 1.0f, 1.0f,		1.0f, 0.0f),
		Axion::Vertex(0.5f,  0.5f, 0.0f,	0.0f, 1.0f, 0.0f, 1.0f,		1.0f, 1.0f),
		Axion::Vertex(-0.5f,  0.5f, 0.0f,	1.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f)
	};
	std::vector<uint32_t> m_quadIndices2 = { 0, 2, 1,	2, 0, 3 };
	Axion::Ref<Axion::Mesh> m_quadMesh2;
	Axion::Ref<Axion::ConstantBuffer> m_quadCB2;
	Axion::Mat4 m_quadTransform2;
	Axion::Vec4 m_quadColor2;

};

class Sandbox : public Axion::Application {
public:

	Sandbox() {
		//pushLayer(new ExampleLayer());
		pushLayer(new Sandbox2D());
	}
	~Sandbox() override {}

};


Axion::Application* Axion::createApplication() {
	return new Sandbox();
}
