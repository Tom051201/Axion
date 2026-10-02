#include "axpch.h"
#include "Skybox.h"

#include "AxionEngine/Source/core/EngineAssets.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/graphics/RenderCommand.h"
#include "AxionEngine/Source/graphics/Renderer.h"
#include "AxionEngine/Source/graphics/GraphicsContext.h"

namespace Axion {

	Skybox::Skybox(AssetHandle<TextureCube> textureHandle)
		: m_textureHandle(textureHandle), m_pipelineHandle(UUID(0, 0)) {}

	Skybox::Skybox(AssetHandle<TextureCube> textureHandle, AssetHandle<Pipeline> pipelineHandle)
		: m_textureHandle(textureHandle), m_pipelineHandle(pipelineHandle) {}

	void Skybox::release() {}

	void Skybox::onUpdate(Timestep ts, RenderContext* rc) {
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

		RenderContext* renderContext = rc;
		if (!renderContext) renderContext = GraphicsContext::get()->getMainRenderContext();

		pipeline->bind(renderContext);

		Renderer::getSceneDataBuffer()->bind(renderContext, 0, Renderer::getSceneDataOffset());
		texture->bind(renderContext, 1);

		Ref<Mesh> mesh = EngineAssets::getCubeMesh();
		mesh->getVertexBuffer()->bind(renderContext);
		mesh->getIndexBuffer()->bind(renderContext);
		RenderCommand::drawIndexed(renderContext, mesh->getVertexBuffer(), mesh->getIndexBuffer());

		pipeline->unbind(renderContext);
	}

}
