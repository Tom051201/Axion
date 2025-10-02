#include "Sandbox3D.h"

Sandbox3D::Sandbox3D() : Layer("Sandbox3D"), m_camera(1280.0f / 720.0f) {}

Sandbox3D::~Sandbox3D() {}

void Sandbox3D::onAttach() {

	//m_meshHandle = Axion::AssetManager::load<Axion::Mesh>("Sandbox/Assets/meshes/cubeBase.obj");
	//
	//m_transform = Axion::Mat4::TRS(Axion::Vec3::zero(), Axion::Vec3::zero(), Axion::Vec3::one());
	//
	//Axion::ShaderSpecification shaderSpec;
	//shaderSpec.name = "Shader3D";
	//shaderSpec.vertexLayout = {
	//	{ "POSITION", Axion::ShaderDataType::Float3 },
	//	{ "COLOR", Axion::ShaderDataType::Float4 },
	//	{ "TEXCOORD", Axion::ShaderDataType::Float2 }
	//};
	//Axion::Ref<Axion::Shader> shader = Axion::Shader::create(shaderSpec);
	////shader->compileFromFile("Sandbox/Assets/shaders/ColorShader.hlsl");
	//shader->compileFromFile("Sandbox/Assets/shaders/PositionShader.hlsl");
	//m_material = Axion::Material::create("BasicMaterial", { 0.0f, 1.0f, 0.0f, 1.0f }, shader);
	//
	//m_buffer = Axion::ConstantBuffer::create(sizeof(Axion::ObjectBuffer));

}

void Sandbox3D::onDetach() {

}

void Sandbox3D::onUpdate(Axion::Timestep ts) {
	//m_camera.onUpdate(ts);
	//
	//Axion::Renderer3D::beginScene(m_camera);
	//
	//Axion::Renderer3D::drawMesh(m_transform, Axion::AssetManager::getMesh(m_meshHandle), m_material, m_buffer);
	//
	//Axion::Renderer3D::endScene();
}

void Sandbox3D::onEvent(Axion::Event& e) {
	//m_camera.onEvent(e);
}

void Sandbox3D::onGuiRender() {}
