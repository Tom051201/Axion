#include "axpch.h"
#include "Renderer.h"

#include "AxionEngine/Source/render/GraphicsContext.h"
#include "AxionEngine/Source/render/RenderCommand.h"
#include "AxionEngine/Source/render/Renderer2D.h"

#include "AxionEngine/Platform/directx/D12Context.h"
#include "AxionEngine/Platform/opengl/OpenGL3Context.h"

namespace Axion {

	struct alignas(16) SceneData {
		DirectX::XMMATRIX viewProjection;
	};

	static SceneData* s_sceneData;
	static Ref<ConstantBuffer> s_sceneUploadBuffer;

	RendererAPI Renderer::s_api = RendererAPI::DirectX12;

	void Renderer::initialize(Window* window) {

		// setup backend specific graphics context
		switch (s_api) {
			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); return; }
			case RendererAPI::DirectX12: { GraphicsContext::set(new D12Context()); break; }
			case RendererAPI::OpenGL3: { GraphicsContext::set(new OpenGL3Context()); break; }
		}
		GraphicsContext::get()->initialize(window->getNativeHandle(), window->getWidth(), window->getHeight());

		// setup scene data
		s_sceneData = new SceneData();
		s_sceneUploadBuffer = ConstantBuffer::create(sizeof(SceneData));
		
		// setup renderer
		Renderer2D::initialize();

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

	void Renderer::beginScene(const Camera& camera) {
		s_sceneData->viewProjection = camera.getViewProjectionMatrix().transposed().toXM();
		s_sceneUploadBuffer->update(s_sceneData, sizeof(SceneData));
	}

	void Renderer::beginScene(const Mat4& projection, const Mat4& transform) {
		Mat4 viewProj = projection * (transform.inverse());
		s_sceneData->viewProjection = viewProj.transposed().toXM();
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

	void Renderer::renderToSwapChain() {
		GraphicsContext::get()->bindSwapChainRenderTarget();
	}

	const Ref<ConstantBuffer>& Renderer::getSceneDataBuffer() {
		return s_sceneUploadBuffer;
	}

	void Renderer::submit(const Ref<Mesh>& mesh, const Ref<ConstantBuffer>& objectData, const Ref<Shader>& shader, const Ref<ConstantBuffer>& uploadBuffer) {
		shader->bind();
		uploadBuffer->bind(0);
		objectData->bind(1);
		mesh->render();
		RenderCommand::drawIndexed(mesh->getVertexBuffer(), mesh->getIndexBuffer());
	}

}

