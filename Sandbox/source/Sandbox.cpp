#include <Axion.h>

class ExampleLayer : public Axion::Layer {
public:

	ExampleLayer() : Layer("Example"), m_cameraController(1280.0f / 720.0f, true) {

		m_shaderLibrary.load("quad", "assets/Shader1.hlsl");
		m_shaderLibrary.load("triangle", "assets/Shader2.hlsl");

		m_quadMesh = Axion::Mesh::create(m_quadVertices, m_quadIndices);
		m_quadCB = Axion::ConstantBuffer::create(sizeof(Axion::ObjectBuffer));
		m_quadTransform = Axion::Mat4::identity();

		m_triangleMesh = Axion::Mesh::create(m_triangleVertices, m_triangleInices);
		m_triangleCB = Axion::ConstantBuffer::create(sizeof(Axion::ObjectBuffer));
		m_triangleTransform = Axion::Mat4::identity();

		m_cameraCB = Axion::ConstantBuffer::create(sizeof(Axion::SceneBuffer));
	}

	void onDetach() override {

		m_shaderLibrary.release();

		m_quadMesh->release();
		m_quadCB->release();

		m_triangleMesh->release();
		m_triangleCB->release();

		m_cameraCB->release();
	}

	void onUpdate(Axion::Timestep ts) override {

		m_cameraController.onUpdate(ts);

		if (Axion::Input::isKeyPressed(Axion::KeyCode::Up)) m_quadTransform.m[0][3] += 1 * ts;
		if (Axion::Input::isKeyPressed(Axion::KeyCode::Down)) m_quadTransform.m[0][3] -= 1 * ts;

		Axion::Renderer::beginScene(m_cameraController.getCamera());
		Axion::Renderer::clear(0.0f, 0.1f, 0.2f, 1.0f);

		// camera
		Axion::SceneBuffer sceneBuffer;
		sceneBuffer.viewProjection = DirectX::XMMatrixTranspose(m_cameraController.getCamera().getViewProjectionMatrix().toXM());
		m_cameraCB->update(&sceneBuffer, sizeof(Axion::SceneBuffer));

		// quad
		m_shaderLibrary.get("quad")->bind();
		Axion::ObjectBuffer quadData;
		quadData.modelMatrix = DirectX::XMMatrixTranspose(m_quadTransform.toXM());
		m_quadCB->update(&quadData, sizeof(Axion::ObjectBuffer));
		Axion::Renderer::submit(m_quadMesh, m_cameraCB, 0, m_quadCB, 1);

		// triangle
		m_shaderLibrary.get("triangle")->bind();
		Axion::ObjectBuffer triangleData;
		triangleData.modelMatrix = DirectX::XMMatrixTranspose(m_triangleTransform.toXM());
		m_triangleCB->update(&triangleData, sizeof(Axion::ObjectBuffer));
		Axion::Renderer::submit(m_triangleMesh, m_cameraCB, 0, m_triangleCB, 1);
	}

	void onEvent(Axion::Event& e) override {
		Axion::EventDispatcher dispatcher(e);
		m_cameraController.onEvent(e);
	}

private:

	Axion::ShaderLibrary m_shaderLibrary;

	Axion::OrthographicCameraController m_cameraController;
	Axion::Ref<Axion::ConstantBuffer> m_cameraCB;

	// triangle
	std::vector<Axion::Vertex> m_triangleVertices = {
		Axion::Vertex(-0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0f),
		Axion::Vertex( 0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0f),
		Axion::Vertex( 0.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0f)
	};

	std::vector<uint32_t> m_triangleInices = { 0, 2, 1 };

	Axion::Ref<Axion::Mesh> m_triangleMesh;
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

	std::vector<uint32_t> m_quadIndices = { 0, 2, 1,	2, 0, 3 };

	Axion::Ref<Axion::Mesh> m_quadMesh;
	Axion::Ref<Axion::ConstantBuffer> m_quadCB;
	Axion::Mat4 m_quadTransform;
	Axion::Vec3 m_quadPosition;
};

class Sandbox : public Axion::Application {
public:

	Sandbox() {
		pushLayer(new ExampleLayer());
		pushLayer(new Axion::ImGuiLayer());
	}
	~Sandbox() override {}

};


Axion::Application* Axion::createApplication() {
	return new Sandbox();
}