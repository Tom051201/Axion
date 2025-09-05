#include "axpch.h"
#include "Skybox.h"

#include "AxionEngine/Source/render/RenderCommand.h"
#include "AxionEngine/Source/render/Renderer.h"

namespace Axion {

	// Simple cube geometry (inside-out normals for skybox)
	static Ref<Mesh> createCubeMesh() {
		std::vector<Vertex> vertices = {
			{ -0.5,  0.5, -0.5 },
			{ -0.5, -0.5, -0.5 },
			{  0.5, -0.5, -0.5 },
			{  0.5,  0.5, -0.5 },
			{ -0.5,  0.5,  0.5 },
			{ -0.5, -0.5,  0.5 },
			{  0.5, -0.5,  0.5 },
			{  0.5,  0.5,  0.5 }
		};

		std::vector<uint32_t> indices = {
			// Back face		// Front face
			0, 1, 2,			4, 7, 6,
			2, 3, 0,			6, 5, 4,

			// Left face		// Right face
			4, 5, 1,			3, 2, 6,
			1, 0, 4,			6, 7, 3,

			// Top face			// Bottom face
			4, 0, 3,			1, 5, 6,
			3, 7, 4,			6, 2, 1
		};

		AssetHandle<Mesh> handle("internal_skybox_mesh");
		return Mesh::create(handle, vertices, indices);
	}

	Skybox::Skybox(const std::string& crossPath) {
		m_mesh = createCubeMesh();
		setTexture(crossPath);
		setupShader("AxionStudio/Assets/shaders/SkyboxShader.hlsl");
	}

	Skybox::Skybox(const std::array<std::string, 6>& facePaths) {
		m_mesh = createCubeMesh();
		m_texture = TextureCube::create(facePaths);
		setupShader("AxionStudio/Assets/shaders/SkyboxShader.hlsl");
	}

	Skybox::~Skybox() {
		release();
	}

	void Skybox::release() {
		m_texture->release();
		m_mesh->release();
		m_shader->release();
	}

	void Skybox::onUpdate(Timestep ts) {
		m_shader->bind();
		Renderer::getSceneDataBuffer()->bind(0);
		m_texture->bind();
		m_mesh->render();
		RenderCommand::drawIndexed(m_mesh->getVertexBuffer(), m_mesh->getIndexBuffer());
		m_shader->unbind();
	}

	void Skybox::setTexture(const std::string& crossPath) {
		m_texture = TextureCube::create(crossPath);
		m_texturePath = crossPath;
	}

	void Skybox::setupShader(const std::string& filePath) {
		ShaderSpecification skySpec;
		skySpec.name = "SkyboxShader";
		skySpec.colorFormat = ColorFormat::RGBA8;
		skySpec.depthStencilFormat = DepthStencilFormat::DEPTH32F;
		skySpec.depthTest = true;
		skySpec.depthWrite = false;
		skySpec.depthFunction = DepthCompare::LessEqual;
		skySpec.cullMode = CullMode::Back;
		skySpec.topology = PrimitiveTopology::TriangleList;
		skySpec.vertexLayout = {
			{ "POSITION", ShaderDataType::Float3 }
		};
		m_shader = Shader::create(skySpec);
		m_shader->compileFromFile(filePath);
	}

}
