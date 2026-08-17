#include "axpch.h"
#include "Skybox.h"

#include "AxionEngine/Source/core/EngineAssets.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/graphics/RenderCommand.h"
#include "AxionEngine/Source/graphics/Renderer.h"

namespace Axion {

	Skybox::Skybox(AssetHandle<TextureCube> textureHandle)
		: m_textureHandle(textureHandle), m_pipelineHandle(UUID(0, 0)) {}

	Skybox::Skybox(AssetHandle<TextureCube> textureHandle, AssetHandle<Pipeline> pipelineHandle)
		: m_textureHandle(textureHandle), m_pipelineHandle(pipelineHandle) {}

	void Skybox::release() {}

	void Skybox::onUpdate(Timestep ts) {
		if (!m_textureHandle.isValid()) return;
		Ref<TextureCube> texture = AssetManager::get<TextureCube>(m_textureHandle);
		if (!texture) return;

		Ref<Pipeline> pipeline;
		if (m_pipelineHandle.isValid()) {
			pipeline = AssetManager::get<Pipeline>(m_pipelineHandle);
		}
		else {
			pipeline = EngineAssets::getSkyboxPipeline();
		}
		if (!pipeline) return;
		pipeline->bind();

		Renderer::getSceneDataBuffer()->bind(0, Renderer::getSceneDataOffset());
		texture->bind(1);

		Ref<Mesh> mesh = EngineAssets::getCubeMesh();
		mesh->getVertexBuffer()->bind();
		mesh->getIndexBuffer()->bind();
		RenderCommand::drawIndexed(mesh->getVertexBuffer(), mesh->getIndexBuffer());

		pipeline->unbind();
	}

}
