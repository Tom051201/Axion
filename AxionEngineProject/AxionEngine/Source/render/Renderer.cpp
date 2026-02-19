#include "axpch.h"
#include "Renderer.h"

#include "AxionEngine/Source/render/GraphicsContext.h"
#include "AxionEngine/Source/render/RenderCommand.h"
#include "AxionEngine/Source/render/Renderer2D.h"
#include "AxionEngine/Source/render/Renderer3D.h"
#include "AxionEngine/Source/events/RenderingEvent.h"

#include "AxionEngine/Platform/directx/D12Context.h"
#include "AxionEngine/Platform/opengl/OpenGL3Context.h"

namespace Axion {

	FrameTimer Renderer::s_frameTimer;
	double Renderer::s_lastFrameTimeMs = 0.0;
	Ref<Texture2D> Renderer::s_whiteFallbackTexture = nullptr;

	struct alignas(16) HLSLDirLight {
		DirectX::XMFLOAT4 direction;
		DirectX::XMFLOAT4 color;
	};

	struct alignas(16) HLSLPointLight {
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT4 params; // x = radius, y = falloff, z = padding, w = padding
	};

	struct alignas(16) HLSLSpotLight {
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT4 direction;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT4 params; // x = range, y = inner cutoff, z = outer cutoff, w = padding
	};

	struct alignas(16) SceneData {
		// TODO: only upload one option! - ViewProjection OR View and Projection
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;

		DirectX::XMMATRIX viewProjection;

		int directionalLightsCount;
		int pointLightsCount;
		int spotLightCount;
		int padding;

		DirectX::XMFLOAT4 ambientColor;

		HLSLDirLight directionalLights[MAX_DIR_LIGHTS];
		HLSLPointLight pointLights[MAX_POINT_LIGHTS];
		HLSLSpotLight spotLights[MAX_SPOT_LIGHTS];
	};


	static SceneData* s_sceneData;
	static Ref<ConstantBuffer> s_sceneUploadBuffer;

	struct RendererData {
		std::function<void(Event&)> eventCallback;
	};

	static RendererData* s_rendererData;

	RendererAPI Renderer::s_api = RendererAPI::DirectX12;

	void Renderer::initialize(Window* window, std::function<void(Event&)> eventCallback) {

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

		// setup renderer data
		s_rendererData = new RendererData();
		s_rendererData->eventCallback = eventCallback;

		// white texture creation
		s_whiteFallbackTexture = Texture2D::create(32, 32, nullptr);

		// setup renderer
		Renderer2D::initialize();
		Renderer3D::initialize();

		AX_CORE_LOG_INFO("Renderer initialized");
	}

	void Renderer::shutdown() {
		delete s_sceneData;
		s_sceneUploadBuffer->release();

		s_whiteFallbackTexture->release();

		// TODO: why are there live obj when doing this???
		// delete s_rendererData;

		Renderer3D::shutdown();
		Renderer2D::shutdown();

		//GraphicsContext::get()->shutdown();	calling this here causes a crash on app termination!!!
		AX_CORE_LOG_INFO("Renderer shutdown");
	}

	void Renderer::prepareRendering() {
		s_frameTimer.begin();
		GraphicsContext::get()->prepareRendering();

		RenderingPreparedEvent ev;
		s_rendererData->eventCallback(ev);

		RenderCommand::clear();
	}

	void Renderer::finishRendering() {
		GraphicsContext::get()->finishRendering();

		s_frameTimer.end();
		s_lastFrameTimeMs = s_frameTimer.getMilliseconds();

		RenderingFinishedEvent ev;
		s_rendererData->eventCallback(ev);
	}

	void Renderer::beginScene(const Camera& camera, const LightingData& lightingData) {
		RenderCommand::resetRenderStats();
		// REVIEW: remove one option
		// option 1: view and projection
		s_sceneData->view = camera.getViewMatrix().transposed().toXM();
		s_sceneData->projection = camera.getProjectionMatrix().transposed().toXM();
		// option 2: viewProjection
		s_sceneData->viewProjection = camera.getViewProjectionMatrix().transposed().toXM();

		// -- Ambient Color --
		s_sceneData->ambientColor = lightingData.ambientColor.toFloat4();

		// -- Directional Lights --
		s_sceneData->directionalLightsCount = std::min((uint32_t)lightingData.directionalLights.size(), MAX_DIR_LIGHTS);
		for (int i = 0; i < s_sceneData->directionalLightsCount; i++) {
			s_sceneData->directionalLights[i].direction = { lightingData.directionalLights[i].direction.x, lightingData.directionalLights[i].direction.y, lightingData.directionalLights[i].direction.z, 1.0f };
			s_sceneData->directionalLights[i].color = lightingData.directionalLights[i].color.toFloat4();
		}

		// -- Point Lights --
		s_sceneData->pointLightsCount = std::min((uint32_t)lightingData.pointLights.size(), MAX_POINT_LIGHTS);
		for (int i = 0; i < s_sceneData->pointLightsCount; i++) {
			s_sceneData->pointLights[i].position = { lightingData.pointLights[i].position.x, lightingData.pointLights[i].position.y, lightingData.pointLights[i].position.z, 0.0f };
			s_sceneData->pointLights[i].color = lightingData.pointLights[i].color.toFloat4();
			s_sceneData->pointLights[i].params = { lightingData.pointLights[i].radius, lightingData.pointLights[i].falloff, 0.0f, 0.0f };
		}

		// -- Spot lights --
		s_sceneData->spotLightCount = std::min((uint32_t)lightingData.spotLights.size(), MAX_SPOT_LIGHTS);
		for (int i = 0; i < s_sceneData->spotLightCount; i++) {
			s_sceneData->spotLights[i].position = { lightingData.spotLights[i].position.x, lightingData.spotLights[i].position.y, lightingData.spotLights[i].position.z, 0.0f };
			s_sceneData->spotLights[i].direction = { lightingData.spotLights[i].direction.x, lightingData.spotLights[i].direction.y, lightingData.spotLights[i].direction.z, 1.0f };
			s_sceneData->spotLights[i].color = lightingData.spotLights[i].color.toFloat4();
			s_sceneData->spotLights[i].params = { lightingData.spotLights[i].range, lightingData.spotLights[i].innerCutoff, lightingData.spotLights[i].outerCutoff, 0.0f };
		}

		s_sceneUploadBuffer->update(s_sceneData, sizeof(SceneData));
	}

	void Renderer::beginScene(const Mat4& projection, const Mat4& transform) {
		Mat4 viewProj = projection * (transform.inverse());
		s_sceneData->viewProjection = viewProj.transposed().toXM();
		s_sceneUploadBuffer->update(s_sceneData, sizeof(SceneData));
	}

	void Renderer::endScene() {
		Renderer2D::endScene();
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

	void Renderer::bindTextures(const std::array<Ref<Texture2D>, 16>& textures, uint32_t count, uint32_t rootIndex) {
		if (s_api == RendererAPI::DirectX12) {
			auto* context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());
			context->bindSrvTable(rootIndex, textures, count);
		}
		else if (s_api == RendererAPI::OpenGL3) {
			// TODO: add opengl3 impl.
			// maybe like this:
			//for (uint32_t i = 0; i < count; i++) {
			//	if (textures[i]) textures[i]->bind(i);
			//}
		}
	}

	const Ref<Texture2D>& Renderer::getWhiteFallbackTexture() {
		return s_whiteFallbackTexture;
	}

	void Renderer::submit(const Ref<Mesh>& mesh, const Ref<ConstantBuffer>& objectData, const Ref<Shader>& shader, const Ref<ConstantBuffer>& uploadBuffer) {
		shader->bind();
		uploadBuffer->bind(0);
		objectData->bind(1);
		mesh->render();
		RenderCommand::drawIndexed(mesh->getVertexBuffer(), mesh->getIndexBuffer());
	}

}
