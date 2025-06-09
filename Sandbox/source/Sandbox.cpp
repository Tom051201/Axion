#include <Axion.h>

// TEMP
#include "platform/directx/D12Shader.h"

class ExampleLayer : public Axion::Layer {
public:

	ExampleLayer() : Layer("Example"), m_camera(-1.6f, 1.6f, -0.9f, 0.9f), m_camPos(0.0f, 0.0f, 0.0f), m_camRot(0.0f) {
		m_shader.reset(Axion::Shader::create("basic"));
		m_shader->compileFromFile("assets/VertexShader.hlsl", "assets/PixelShader.hlsl");
		
		m_shader2.reset(Axion::Shader::create("basic2"));
		m_shader2->compileFromFile("assets/VertexShader2.hlsl", "assets/PixelShader2.hlsl");

		m_quadVB.reset(Axion::VertexBuffer::create(m_quadVertices));
		m_quadIB.reset(Axion::IndexBuffer::create(m_quadIndices));
		m_quadCB.reset(Axion::ConstantBuffer::create(sizeof(Axion::ObjectBuffer)));
		m_quadTransform = Axion::Mat4::identity();

		m_triangleVB.reset(Axion::VertexBuffer::create(m_triangleVertices));
		m_triangleIB.reset(Axion::IndexBuffer::create(m_triangleInices));
		m_triangleCB.reset(Axion::ConstantBuffer::create(sizeof(Axion::ObjectBuffer)));
		m_triangleTransform = Axion::Mat4::identity();

		m_cameraCB.reset(Axion::ConstantBuffer::create(sizeof(Axion::SceneBuffer)));
	}

	void onDetach() override {

		m_shader->release();
		m_shader2->release();

		m_quadVB->release();
		m_quadIB->release();
		m_quadCB->release();

		m_triangleVB->release();
		m_triangleIB->release();
		m_triangleCB->release();

		m_cameraCB->release();

	}

	void onUpdate(Axion::Timestep ts) override {

		if (Axion::Input::isKeyPressed(Axion::KeyCode::Left)) m_camPos.x -= m_camMoveSpeed * ts;
		if (Axion::Input::isKeyPressed(Axion::KeyCode::Right)) m_camPos.x += m_camMoveSpeed * ts;
		if (Axion::Input::isKeyPressed(Axion::KeyCode::Up)) m_camPos.y += m_camMoveSpeed * ts;
		if (Axion::Input::isKeyPressed(Axion::KeyCode::Down)) m_camPos.y -= m_camMoveSpeed * ts;
		if (Axion::Input::isKeyPressed(Axion::KeyCode::A)) m_camRot += m_camRotSpeed * ts;
		if (Axion::Input::isKeyPressed(Axion::KeyCode::D)) m_camRot -= m_camRotSpeed * ts;
		if (Axion::Input::isKeyPressed(Axion::KeyCode::W)) m_quadTransform.m[0][3] += 1 * ts;
		if (Axion::Input::isKeyPressed(Axion::KeyCode::S)) m_quadTransform.m[0][3] -= 1 * ts;

		m_camera.setPosition(m_camPos);
		m_camera.setRotationZ(m_camRot);

		Axion::Renderer::beginScene(m_camera);
		Axion::Renderer::clear(0.0f, 0.1f, 0.2f, 1.0f);

		// camera
		Axion::SceneBuffer sceneBuffer;
		sceneBuffer.viewProjection = DirectX::XMMatrixTranspose(m_camera.getViewProjectionMatrix().toXM());
		m_cameraCB->update(&sceneBuffer, sizeof(Axion::SceneBuffer));

		// quad
		m_shader->bind();
		Axion::ObjectBuffer quadData;
		quadData.modelMatrix = DirectX::XMMatrixTranspose(m_quadTransform.toXM());
		m_quadCB->update(&quadData, sizeof(Axion::ObjectBuffer));
		Axion::Renderer::submit(m_quadVB, m_quadIB, m_cameraCB, 0, m_quadCB, 1);

		// triangle
		m_shader2->bind();
		Axion::ObjectBuffer triangleData;
		triangleData.modelMatrix = DirectX::XMMatrixTranspose(m_triangleTransform.toXM());
		m_triangleCB->update(&triangleData, sizeof(Axion::ObjectBuffer));
		Axion::Renderer::submit(m_triangleVB, m_triangleIB, m_cameraCB, 0, m_triangleCB, 1);
	}

	void onEvent(Axion::Event& e) override {
		Axion::EventDispatcher dispatcher(e);
	}

private:

	Axion::OrthographicCamera m_camera;
	Axion::Vec3 m_camPos;
	Axion::Ref<Axion::ConstantBuffer> m_cameraCB;
	float m_camRot;
	float m_camMoveSpeed = 1.0f;
	float m_camRotSpeed = 2.0f;

	// triangle
	std::vector<Axion::Vertex> m_triangleVertices = {
		Axion::Vertex(-0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0f),
		Axion::Vertex( 0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0f),
		Axion::Vertex( 0.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0f)
	};

	std::vector<uint32_t> m_triangleInices = {
		0, 2, 1
	};

	Axion::Ref<Axion::VertexBuffer> m_triangleVB;
	Axion::Ref<Axion::IndexBuffer> m_triangleIB;
	Axion::Ref<Axion::ConstantBuffer> m_triangleCB;
	Axion::Mat4 m_triangleTransform;
	Axion::Vec3 m_trianglePosition;



	// quad
	std::vector<Axion::Vertex> m_quadVertices = {
		Axion::Vertex(-0.5f, -0.5f, 0.0f,	1.0f, 1.0f, 0.0f, 1.0f,		0.0f, 0.0f),
		Axion::Vertex( 0.5f, -0.5f, 0.0f,	0.0f, 0.0f, 1.0f, 1.0f,		1.0f, 0.0f),
		Axion::Vertex( 0.5f,  0.5f, 0.0f,	0.0f, 1.0f, 0.0f, 1.0f,		1.0f, 1.0f),
		Axion::Vertex(-0.5f,  0.5f, 0.0f,	1.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f)
	};

	std::vector<uint32_t> m_quadIndices = {
		0, 2, 1,
		2, 0, 3
	};

	Axion::Ref<Axion::VertexBuffer> m_quadVB;
	Axion::Ref<Axion::IndexBuffer> m_quadIB;
	Axion::Ref<Axion::ConstantBuffer> m_quadCB;
	Axion::Mat4 m_quadTransform;
	Axion::Vec3 m_quadPosition;


	Axion::Ref<Axion::Shader> m_shader;
	Axion::Ref<Axion::Shader> m_shader2;
};

class Sandbox : public Axion::Application {
public:

	Sandbox() {
		pushLayer(new ExampleLayer());
		pushLayer(new Axion::ImGuiLayer());
	}
	~Sandbox() override {

	}

};


Axion::Application* Axion::createApplication() {
	return new Sandbox();
}