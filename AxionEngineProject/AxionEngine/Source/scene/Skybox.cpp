#include "axpch.h"
#include "Skybox.h"

#include "AxionEngine/Source/render/RenderCommand.h"

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
			// Back face
			0, 1, 2,
			2, 3, 0,

			// Front face
			4, 7, 6,
			6, 5, 4,

			// Left face
			4, 5, 1,
			1, 0, 4,

			// Right face
			3, 2, 6,
			6, 7, 3,

			// Top face
			4, 0, 3,
			3, 7, 4,

			// Bottom face
			1, 5, 6,
			6, 2, 1
		};

		AssetHandle<Mesh> handle("");
		return Mesh::create(handle, vertices, indices);
	}

	Skybox::Skybox(const std::array<std::string, 6>& facePaths) {
		m_mesh = createCubeMesh();
		m_texture = TextureCube::create(facePaths);

		ShaderSpecification skySpec;
		skySpec.name = "SkyboxShader";
		skySpec.colorFormat = ColorFormat::RGBA8;
		skySpec.depthStencilFormat = DepthStencilFormat::DEPTH32F;
		skySpec.depthTest = true;
		skySpec.depthWrite = false;
		skySpec.depthFunction = DepthCompare::LessEqual;
		skySpec.cullMode = CullMode::Front;
		skySpec.topology = PrimitiveTopology::TriangleList;
		skySpec.vertexLayout = {
			{ "POSITION", ShaderDataType::Float3 }
		};
		m_shader = Shader::create(skySpec);
		m_shader->compileFromFile("AxionStudio/Assets/shaders/SkyboxShader.hlsl");
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
		m_texture->bind();
		m_mesh->render();
		RenderCommand::drawIndexed(m_mesh->getVertexBuffer(), m_mesh->getIndexBuffer());

		m_shader->unbind();
	}

}
