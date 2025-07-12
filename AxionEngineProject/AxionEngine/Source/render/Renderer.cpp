#include "axpch.h"
#include "Renderer.h"

#include "AxionEngine/Source/render/GraphicsContext.h"
#include "AxionEngine/Source/render/RenderCommand.h"
#include "AxionEngine/Source/render/Renderer2D.h"

namespace Axion {

	struct alignas(16) SceneData {
		DirectX::XMMATRIX viewProjection;
	};

	static SceneData* s_sceneData;
	static Ref<ConstantBuffer> s_sceneUploadBuffer;

	RendererAPI Renderer::s_api = RendererAPI::DirectX12;

	void Renderer::initialize(Window* window) {
		GraphicsContext::get()->initialize(window->getNativeHandle(), window->getWidth(), window->getHeight());


		Renderer2D::initialize();

		s_sceneData = new SceneData();
		s_sceneUploadBuffer = ConstantBuffer::create(sizeof(SceneData));

		AX_CORE_LOG_INFO("Renderer initialized");
	}

	void Renderer::release() {
		delete s_sceneData;
		s_sceneUploadBuffer->release();

		Renderer2D::shutdown();

		//GraphicsContext::get()->shutdown();	calling this here causes a crash on app termination!!!
		AX_CORE_LOG_INFO("Renderer shutdown");
	}

	void Renderer::prepareRendering() {
		GraphicsContext::get()->prepareRendering();
	}

	void Renderer::finishRendering() {
		GraphicsContext::get()->finishRendering();
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

