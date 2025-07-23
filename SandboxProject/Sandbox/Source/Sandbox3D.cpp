#include "Sandbox3D.h"

Sandbox3D::Sandbox3D() : Layer("Sandbox3D"), m_camera(1280.0f / 720.0f) {}

Sandbox3D::~Sandbox3D() {}

void Sandbox3D::onAttach() {

	m_transform = Axion::Mat4::TRS(Axion::Vec3::zero(), Axion::Vec3::zero(), Axion::Vec3::one());

	std::vector<Axion::Vertex> vertices = {
		// Front face
		Axion::Vertex(-0.5f, -0.5f,  0.5f,   1, 0, 0, 1,   0.0f, 0.0f),
		Axion::Vertex( 0.5f, -0.5f,  0.5f,   0, 1, 0, 1,   1.0f, 0.0f),
		Axion::Vertex( 0.5f,  0.5f,  0.5f,   0, 0, 1, 1,   1.0f, 1.0f),
		Axion::Vertex(-0.5f,  0.5f,  0.5f,   1, 1, 0, 1,   0.0f, 1.0f),

		// Back face
		Axion::Vertex(-0.5f, -0.5f, -0.5f,   1, 0, 1, 1,   1.0f, 0.0f),
		Axion::Vertex( 0.5f, -0.5f, -0.5f,   0, 1, 1, 1,   0.0f, 0.0f),
		Axion::Vertex( 0.5f,  0.5f, -0.5f,   1, 1, 1, 1,   0.0f, 1.0f),
		Axion::Vertex(-0.5f,  0.5f, -0.5f,   0, 0, 0, 1,   1.0f, 1.0f)
	};
	std::vector<uint32_t> indices = {
		// Front face
		0, 1, 2,
		2, 3, 0,

		// Right face
		1, 5, 6,
		6, 2, 1,

		// Back face
		5, 4, 7,
		7, 6, 5,

		// Left face
		4, 0, 3,
		3, 7, 4,

		// Top face
		3, 2, 6,
		6, 7, 3,

		// Bottom face
		4, 5, 1,
		1, 0, 4
	};
	m_mesh = Axion::Mesh::create(vertices, indices);

	Axion::ShaderSpecification shaderSpec;
	shaderSpec.name = "Shader3D";
	shaderSpec.vertexLayout = {
		{ "POSITION", Axion::ShaderDataType::Float3 },
		{ "COLOR", Axion::ShaderDataType::Float4 },
		{ "TEXCOORD", Axion::ShaderDataType::Float2 }
	};
	Axion::Ref<Axion::Shader> shader = Axion::Shader::create(shaderSpec);
	shader->compileFromFile("Sandbox/Assets/shaders/ColorShader.hlsl");
	m_material = Axion::Material::create("BasicMaterial", { 0.0f, 0.0f, 1.0f, 1.0f }, shader);

	m_buffer = Axion::ConstantBuffer::create(sizeof(Axion::ObjectBuffer));

}

void Sandbox3D::onDetach() {

}

void Sandbox3D::onUpdate(Axion::Timestep ts) {
	m_camera.onUpdate(ts);

	Axion::Renderer3D::beginScene(m_camera);

	Axion::Renderer3D::drawMesh(m_transform, m_mesh, m_material, m_buffer);

	Axion::Renderer3D::endScene();
}

void Sandbox3D::onEvent(Axion::Event& e) {
	m_camera.onEvent(e);
}

void Sandbox3D::onGuiRender() {}
