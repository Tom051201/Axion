#include "axpch.h"
#include "Renderer.h"

#include "platform/directx/D12RendererAPI.h"

namespace Axion {

	struct alignas(16) SceneData {
		DirectX::XMMATRIX viewProjection;
	};

	static SceneData* s_sceneData;
	static Ref<ConstantBuffer> s_uploadBuffer;

	RendererAPI* Renderer::s_rendererAPI = new D12RendererAPI();

	void Renderer::initialize(Window* window) {
		s_rendererAPI->initialize(window);

		s_sceneData = new SceneData();
		s_uploadBuffer = ConstantBuffer::create(sizeof(SceneData));

		AX_CORE_LOG_INFO("Renderer initialized");
	}

	void Renderer::release() {
		delete s_sceneData;
		s_uploadBuffer->release();

		s_rendererAPI->release();
		AX_CORE_LOG_INFO("Renderer shutdown");
	}

	void Renderer::beginScene(OrthographicCamera& camera) {
		s_rendererAPI->beginScene();
		
		s_sceneData->viewProjection = DirectX::XMMatrixTranspose(camera.getViewProjectionMatrix().toXM());
		s_uploadBuffer->update(s_sceneData, sizeof(SceneData));
	}

	void Renderer::endScene() {
		s_rendererAPI->endScene();
	}

	void Renderer::clear(float r, float g, float b, float a) {
		s_rendererAPI->clear(r, g, b, a);
	}

	void Renderer::present() {
		s_rendererAPI->present();
	}

	void Renderer::submit(const Ref<Mesh>& mesh, const Ref<ConstantBuffer>& objectData, const Ref<Shader>& shader) {
		shader->bind();
		s_uploadBuffer->bind(0);
		objectData->bind(1);
		mesh->render();
		s_rendererAPI->drawIndexed(mesh->getVertexBuffer(), mesh->getIndexBuffer());
	}

}

