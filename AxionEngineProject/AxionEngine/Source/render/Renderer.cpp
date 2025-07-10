#include "axpch.h"
#include "Renderer.h"

#include "AxionEngine/Source/render/RenderCommand.h"
#include "AxionEngine/Source/render/Renderer2D.h"

#include "AxionEngine/Platform/directx/D12RendererAPI.h"

namespace Axion {

	struct alignas(16) SceneData {
		DirectX::XMMATRIX viewProjection;
	};

	static SceneData* s_sceneData;
	static Ref<ConstantBuffer> s_sceneUploadBuffer;

	RendererAPI* Renderer::s_rendererAPI = new D12RendererAPI();

	void Renderer::initialize(Window* window) {
		s_rendererAPI->initialize(window);
	
		Renderer2D::initialize();

		s_sceneData = new SceneData();
		s_sceneUploadBuffer = ConstantBuffer::create(sizeof(SceneData));

		AX_CORE_LOG_INFO("Renderer initialized");
	}

	void Renderer::release() {
		delete s_sceneData;
		s_sceneUploadBuffer->release();

		Renderer2D::shutdown();

		s_rendererAPI->release();
		AX_CORE_LOG_INFO("Renderer shutdown");
	}

	void Renderer::prepareRendering() {
		s_rendererAPI->prepareRendering();
	}

	void Renderer::finishRendering() {
		s_rendererAPI->finishRendering();
	}

	void Renderer::beginScene(OrthographicCamera& camera) {
		s_sceneData->viewProjection = DirectX::XMMatrixTranspose(camera.getViewProjectionMatrix().toXM());
		s_sceneUploadBuffer->update(s_sceneData, sizeof(SceneData));
	}

	void Renderer::endScene() {
		// Does nothing for now
	}

	void Renderer::setClearColor(const Vec4& color) {
		RenderCommand::setClearColor(color);
	}

	void Renderer::clear() {
		RenderCommand::clear();
	}

	void Renderer::submit(const Ref<Mesh>& mesh, const Ref<ConstantBuffer>& objectData, const Ref<Shader>& shader) {
		shader->bind();
		s_sceneUploadBuffer->bind(0);
		objectData->bind(1);
		mesh->render();
		RenderCommand::drawIndexed(mesh->getVertexBuffer(), mesh->getIndexBuffer());
	}

}

