#include "axpch.h"
#include "Renderer.h"

#include "AxionEngine/Source/graphics/GraphicsContext.h"
#include "AxionEngine/Source/graphics/RenderCommand.h"
#include "AxionEngine/Source/graphics/Renderer2D.h"
#include "AxionEngine/Source/graphics/Renderer3D.h"
#include "AxionEngine/Source/events/RenderingEvent.h"

#include "AxionEngine/Platform/directx11/DX11Context.h"
#include "AxionEngine/Platform/directx12/DX12Context.h"

namespace Axion {

	RendererStats Renderer::s_stats;
	FrameTimer Renderer::s_frameTimer;
	double Renderer::s_lastFrameTimeMs = 0.0;
	std::function<void(Event&)> Renderer::s_eventCallback;
	thread_local uint32_t Renderer::s_sceneDataOffset = 0;
	Ref<Texture2D> Renderer::s_shadowMapTexture = nullptr;
	FrameBuffer* Renderer::s_currentRenderTarget = nullptr;
	Mat4 Renderer::s_lightView;
	Mat4 Renderer::s_lightProjection;

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
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;

		DirectX::XMMATRIX viewProjection;
		DirectX::XMMATRIX lightSpaceMatrix;

		int directionalLightsCount;
		int pointLightsCount;
		int spotLightCount;
		int padding;

		DirectX::XMFLOAT4 ambientColor;

		HLSLDirLight directionalLights[MAX_DIR_LIGHTS];
		HLSLPointLight pointLights[MAX_POINT_LIGHTS];
		HLSLSpotLight spotLights[MAX_SPOT_LIGHTS];
	};


	static Ref<ConstantBuffer> s_sceneUploadBuffer;

	RendererAPI Renderer::s_api = RendererAPI::DirectX11;

	void Renderer::initialize(Window* window, std::function<void(Event&)> eventCallback) {
		s_eventCallback = eventCallback;

		// -- Setup Backend specific graphics context --
		switch (s_api) {
			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); return; }
			case RendererAPI::DirectX12: { GraphicsContext::set(new DX12Context()); break; }
			case RendererAPI::DirectX11: { GraphicsContext::set(new DX11Context()); break; }
		}
		GraphicsContext::get()->initialize(window->getNativeHandle(), window->getWidth(), window->getHeight());

		// -- Setup scene data --
		const uint32_t MaxScenePasses = 100;
		uint32_t alignedSceneDataSize = (sizeof(SceneData) + 255) & ~255;
		s_sceneUploadBuffer = ConstantBuffer::create(alignedSceneDataSize * MaxScenePasses);

		// -- Setup shadow map texture --
		s_shadowMapTexture = DepthTexture::create(2048, 2048);

		AX_CORE_LOG_INFO("Renderer initialized");
	}

	void Renderer::shutdown() {
		s_sceneUploadBuffer->release();
		s_shadowMapTexture->release();

		GraphicsContext::get()->shutdown();
		AX_CORE_LOG_INFO("Renderer shutdown");
	}

	void Renderer::prepareRendering() {
		resetStats();
		s_frameTimer.begin();
		GraphicsContext::get()->prepareRendering();

		if (s_sceneUploadBuffer) {
			s_sceneUploadBuffer->resetOffset();
		}

		Renderer2D::beginFrame();
		Renderer3D::beginFrame();

		RenderingPreparedEvent ev;
		s_eventCallback(ev);

		RenderContext* renderContext = GraphicsContext::get()->getMainRenderContext();
		RenderCommand::clear(renderContext);
	}

	void Renderer::finishRendering() {
		GraphicsContext::get()->finishRendering();

		s_frameTimer.end();
		s_lastFrameTimeMs = s_frameTimer.getMilliseconds();

		RenderingFinishedEvent ev;
		s_eventCallback(ev);
	}

	RendererStats& Renderer::getStats() {
		return s_stats;
	}

	void Renderer::resetStats() {
		memset(&s_stats, 0, sizeof(RendererStats));
	}

	void Renderer::beginScene(const Camera& camera, const LightingData& lightingData) {
		SceneData sceneData;
		// REVIEW: remove one option
		// option 1: view and projection
		sceneData.view = camera.getViewMatrix().transposed().toXM();
		sceneData.projection = camera.getProjectionMatrix().transposed().toXM();
		// option 2: viewProjection
		sceneData.viewProjection = camera.getViewProjectionMatrix().transposed().toXM();

		// -- Ambient Color --
		sceneData.ambientColor = lightingData.ambientColor.toFloat4();

		// -- Directional Lights --
		sceneData.directionalLightsCount = std::min((uint32_t)lightingData.directionalLights.size(), MAX_DIR_LIGHTS);
		for (int i = 0; i < sceneData.directionalLightsCount; i++) {
			sceneData.directionalLights[i].direction = { lightingData.directionalLights[i].direction.x, lightingData.directionalLights[i].direction.y, lightingData.directionalLights[i].direction.z, 1.0f };
			sceneData.directionalLights[i].color = lightingData.directionalLights[i].color.toFloat4();
		}

		// -- Light Space Matrix for Shadows --
		if (sceneData.directionalLightsCount > 0) {
			Vec3 lightDir = {
				sceneData.directionalLights[0].direction.x,
				sceneData.directionalLights[0].direction.y,
				sceneData.directionalLights[0].direction.z
			};

			Mat4 camInv = camera.getViewMatrix().inverse();
			Vec3 camPos = { camInv.data()[12], camInv.data()[13], camInv.data()[14] };

			float lightDistance = 100.0f;
			Vec3 lightPos = camPos + (lightDir * lightDistance);
			Vec3 lightUp = (std::abs(lightDir.y) > 0.99f) ? Vec3(0.0f, 0.0f, 1.0f) : Vec3(0.0f, 1.0f, 0.0f);

			s_lightView = Mat4::lookAt(lightPos, camPos, lightUp);

			float orthoSize = 60.0f;
			float zNear = 1.0f;
			float zFar = 250.0f;
			s_lightProjection = Mat4::orthographicOffCenter(-orthoSize, orthoSize, -orthoSize, orthoSize, zNear, zFar);

			Mat4 lightSpace = s_lightProjection * s_lightView;
			sceneData.lightSpaceMatrix = lightSpace.transposed().toXM();
		}
		else {
			s_lightView = Mat4::identity();
			s_lightProjection = Mat4::identity();
			sceneData.lightSpaceMatrix = Mat4::identity().toXM();
		}

		// -- Point Lights --
		sceneData.pointLightsCount = std::min((uint32_t)lightingData.pointLights.size(), MAX_POINT_LIGHTS);
		for (int i = 0; i < sceneData.pointLightsCount; i++) {
			sceneData.pointLights[i].position = { lightingData.pointLights[i].position.x, lightingData.pointLights[i].position.y, lightingData.pointLights[i].position.z, 0.0f };
			sceneData.pointLights[i].color = lightingData.pointLights[i].color.toFloat4();
			sceneData.pointLights[i].params = { lightingData.pointLights[i].radius, lightingData.pointLights[i].falloff, 0.0f, 0.0f };
		}

		// -- Spot lights --
		sceneData.spotLightCount = std::min((uint32_t)lightingData.spotLights.size(), MAX_SPOT_LIGHTS);
		for (int i = 0; i < sceneData.spotLightCount; i++) {
			sceneData.spotLights[i].position = { lightingData.spotLights[i].position.x, lightingData.spotLights[i].position.y, lightingData.spotLights[i].position.z, 0.0f };
			sceneData.spotLights[i].direction = { lightingData.spotLights[i].direction.x, lightingData.spotLights[i].direction.y, lightingData.spotLights[i].direction.z, 1.0f };
			sceneData.spotLights[i].color = lightingData.spotLights[i].color.toFloat4();
			sceneData.spotLights[i].params = { lightingData.spotLights[i].range, lightingData.spotLights[i].innerCutoff, lightingData.spotLights[i].outerCutoff, 0.0f };
		}

		s_sceneDataOffset = s_sceneUploadBuffer->append(&sceneData, sizeof(SceneData));
	}

	void Renderer::beginScene(const Mat4& projection, const Mat4& transform) {
		SceneData sceneData;
		Mat4 viewProj = projection * (transform.inverse());
		sceneData.viewProjection = viewProj.transposed().toXM();
		s_sceneDataOffset = s_sceneUploadBuffer->append(&sceneData, sizeof(SceneData));
	}

	void Renderer::endScene() {
		RenderContext* context = GraphicsContext::get()->getMainRenderContext();
		Renderer2D::endScene(context);
	}

	void Renderer::setClearColor(RenderContext* renderContext, const Vec4& color) {
		RenderCommand::setClearColor(color);
	}

	void Renderer::clear(RenderContext* renderContext) {
		RenderCommand::clear(renderContext);
	}

	void Renderer::setRenderTarget(RenderContext* renderContext, FrameBuffer* target) {
		s_currentRenderTarget = target;
	}

	void Renderer::restoreRenderTarget(RenderContext* renderContext) {
		if (s_currentRenderTarget) {
			s_currentRenderTarget->bind(renderContext);
		}
		else {
			renderToSwapChain(renderContext);
		}
	}

	void Renderer::renderToSwapChain(RenderContext* renderContext) {
		GraphicsContext::get()->bindSwapChainRenderTarget();
	}

	const Ref<ConstantBuffer>& Renderer::getSceneDataBuffer() {
		return s_sceneUploadBuffer;
	}

	uint32_t Renderer::getSceneDataOffset() {
		return s_sceneDataOffset;
	}

	void Renderer::bindTextures(RenderContext* renderContext, const std::array<Ref<Texture2D>, 16>& textures, uint32_t count, uint32_t rootIndex) {
		std::array<Ref<Texture2D>, 16> finalTextures = textures;

		if (s_shadowMapTexture) {
			finalTextures[6] = s_shadowMapTexture;
			if (count < 7) count = 7;
		}

		if (s_api == RendererAPI::DirectX12) { // TODO: abstract those checks away
			auto* dx12Context = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext());
			dx12Context->bindSrvTable(renderContext, rootIndex, finalTextures, count);
		}
		else if (s_api == RendererAPI::DirectX11) {
			auto* context = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext());
			context->bindSrvTable(rootIndex, finalTextures, count);
		}
	}

	Ref<Texture2D> Renderer::getShadowMap() {
		return s_shadowMapTexture;
	}

	void Renderer::submit(RenderContext* renderContext, const Ref<Mesh>& mesh, const Ref<ConstantBuffer>& objectData, const Ref<Shader>& shader, const Ref<ConstantBuffer>& uploadBuffer) {
		if (!mesh || !shader || !objectData || !uploadBuffer) return;

		shader->bind(renderContext);
		uploadBuffer->bind(renderContext, 0);
		objectData->bind(renderContext, 1);
		mesh->getVertexBuffer()->bind(renderContext);
		mesh->getIndexBuffer()->bind(renderContext);
		RenderCommand::drawIndexed(renderContext, mesh->getVertexBuffer(), mesh->getIndexBuffer());
	}

}
