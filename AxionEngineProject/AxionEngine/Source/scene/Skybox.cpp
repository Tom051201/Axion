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

		return Mesh::create(vertices, indices);
	}

	Skybox::Skybox(AssetHandle<TextureCube> textureHandle, AssetHandle<Pipeline> pipelineHandle)
		: m_textureHandle(textureHandle), m_pipelineHandle(pipelineHandle) {
		m_mesh = createCubeMesh();
	}

	Skybox::~Skybox() {
		release();
	}

	void Skybox::release() {
		m_mesh->release();
	}

	void Skybox::onUpdate(Timestep ts) {
		if (!m_pipelineHandle.isValid() || !m_textureHandle.isValid()) return;

		Ref<Pipeline> pipeline = AssetManager::get<Pipeline>(m_pipelineHandle);
		Ref<TextureCube> texture = AssetManager::get<TextureCube>(m_textureHandle);

		if (!pipeline || !texture) return;

		pipeline->bind();
		Renderer::getSceneDataBuffer()->bind(0);
		texture->bind(1);

		m_mesh->render();
		RenderCommand::drawIndexed(m_mesh->getVertexBuffer(), m_mesh->getIndexBuffer());

		pipeline->unbind();
	}

}
