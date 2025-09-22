#include "axpch.h"
#include "Skybox.h"

#include "AxionEngine/Source/render/RenderCommand.h"
#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/core/AssetManager.h"

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

		AssetHandle<Mesh> handle;
		return Mesh::create(handle, vertices, indices);
	}

	Skybox::Skybox(const std::string& crossPath) {
		m_mesh = createCubeMesh();
		setTexture(crossPath);
		setupShader("shaders/skyboxShader.axshader");
	}

	Skybox::Skybox(const std::array<std::string, 6>& facePaths) {
		m_mesh = createCubeMesh();
		m_texture = TextureCube::create(facePaths);
		setupShader("shaders/skyboxShader.axshader");
	}

	Skybox::~Skybox() {
		release();
	}

	void Skybox::release() {
		m_texture->release();
		m_mesh->release();
		//m_shader->release();
	}

	void Skybox::onUpdate(Timestep ts) {
		AssetManager::get<Shader>(m_shaderHandle)->bind();
		Renderer::getSceneDataBuffer()->bind(0);
		m_texture->bind();
		m_mesh->render();
		RenderCommand::drawIndexed(m_mesh->getVertexBuffer(), m_mesh->getIndexBuffer());
		AssetManager::get<Shader>(m_shaderHandle)->unbind();
	}

	void Skybox::setTexture(const std::string& crossPath) {
		m_texture = TextureCube::create(crossPath);
		m_texturePath = crossPath;
	}

	void Skybox::setupShader(const std::string& filePath) {
		AssetHandle<Shader> handle = AssetManager::load<Shader>(AssetManager::getAbsolute(filePath));
		m_shaderHandle = handle;
	}

}
