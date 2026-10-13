#include "axpch.h"
#include "Skybox.h"

#include "AxionEngine/Source/core/EngineAssets.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/graphics/RenderCommand.h"
#include "AxionEngine/Source/graphics/Renderer.h"
#include "AxionEngine/Source/graphics/GraphicsContext.h"
#include "AxionEngine/Source/graphics/RenderContext.h"

namespace Axion {

	Skybox::Skybox(AssetHandle<TextureCube> textureHandle)
		: m_textureHandle(textureHandle), m_pipelineHandle(UUID(0, 0)) {
		m_skyboxBuffer = ConstantBuffer::create(sizeof(int) * 4);
	}

	Skybox::Skybox(AssetHandle<TextureCube> textureHandle, AssetHandle<Pipeline> pipelineHandle)
		: m_textureHandle(textureHandle), m_pipelineHandle(pipelineHandle) {
		m_skyboxBuffer = ConstantBuffer::create(sizeof(int) * 4);
	}

	void Skybox::release() {
		if (m_skyboxBuffer) m_skyboxBuffer->release();
	}

	void Skybox::onUpdate(Timestep ts, RenderContext* renderContext) {
		if (!m_textureHandle.isValid()) return;
		Ref<TextureCube> texture = AssetManager::get<TextureCube>(m_textureHandle);
		if (!texture) return;

		Ref<Pipeline> pipeline;
		if (m_pipelineHandle.isValid()) pipeline = AssetManager::get<Pipeline>(m_pipelineHandle);
		else pipeline = EngineAssets::getSkyboxPipeline();
		if (!pipeline) return;

		if (!renderContext) renderContext = GraphicsContext::get()->getMainRenderContext();

		pipeline->bind(renderContext);

		renderContext->bindGlobalTextureTable((uint32_t)RootSlot::Textures);
		Renderer::getSceneDataBuffer()->bind(renderContext, (uint32_t)RootSlot::Camera, Renderer::getSceneDataOffset());

		int skyboxData[4] = { (int)texture->getBindlessIndex(), 0, 0, 0 };
		m_skyboxBuffer->update(skyboxData, sizeof(skyboxData));
		m_skyboxBuffer->bind(renderContext, (uint32_t)RootSlot::Material);

		Ref<Mesh> mesh = EngineAssets::getCubeMesh();
		mesh->getVertexBuffer()->bind(renderContext);
		mesh->getIndexBuffer()->bind(renderContext);
		RenderCommand::drawIndexed(renderContext, mesh->getVertexBuffer(), mesh->getIndexBuffer());
	}

}
