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
	RendererConfig Renderer::s_config;
	FrameTimer Renderer::s_frameTimer;
	double Renderer::s_lastFrameTimeMs = 0.0;
	std::function<void(Event&)> Renderer::s_eventCallback;
	thread_local uint32_t Renderer::s_sceneDataOffset = 0;
	FrameBuffer* Renderer::s_currentRenderTarget = nullptr;
	std::array<Ref<Texture2D>, 4> Renderer::s_shadowMapTextures;
	std::array<Mat4, 4> Renderer::s_lightSpaceMatrices;

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
		DirectX::XMMATRIX viewProjection;
		DirectX::XMMATRIX skyboxViewProjection;
		DirectX::XMMATRIX lightSpaceMatrix;
		DirectX::XMFLOAT4 cameraPosition;
		DirectX::XMFLOAT4 ambientColor;

		int hasDirectionalLight;
		int pointLightsCount;
		int spotLightCount;
		int shadowMapIndex;

		int skyboxIndex;
		int padding1;
		int padding2;
		int padding3;

		HLSLDirLight directionalLight;
		HLSLPointLight pointLights[MAX_POINT_LIGHTS];
		HLSLSpotLight spotLights[MAX_SPOT_LIGHTS];

		DirectX::XMMATRIX lightSpaceMatrices[4];
		DirectX::XMFLOAT4 cascadeSplits;
		DirectX::XMFLOAT4 cascadeRadii;
		DirectX::XMINT4 shadowMapIndices;
		DirectX::XMINT4 shadowParams; // x = shadowQuality (0..4), y = activeCascades, z = unused, w = unused
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

		// -- Setup 4 Cascade Shadow Map Textures --
		for (int i = 0; i < 4; i++) {
			s_shadowMapTextures[i] = DepthTexture::create(2048, 2048);
		}

		AX_CORE_LOG_INFO("Renderer initialized");
	}

	void Renderer::shutdown() {
		s_sceneUploadBuffer->release();
		for (int i = 0; i < 4; i++) {
			s_shadowMapTextures[i]->release();
		}

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
		SceneData sceneData{};

		// -- Standard View-Projection --
		sceneData.viewProjection = camera.getViewProjectionMatrix().transposed().toXM();

		// -- Skybox View-Projection --
		DirectX::XMMATRIX viewXM = camera.getViewMatrix().toXM();
		viewXM.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		Mat4 skyboxVP = Mat4::fromXM(viewXM) * camera.getProjectionMatrix();
		sceneData.skyboxViewProjection = skyboxVP.transposed().toXM();

		// -- Pre-calculated camera position --
		Mat4 camInv = camera.getViewMatrix().inverse();
		Vec3 camPos = { camInv.data()[12], camInv.data()[13], camInv.data()[14] };
		sceneData.cameraPosition = { camPos.x, camPos.y, camPos.z, 1.0f };

		// -- Ambient Color --
		sceneData.ambientColor = lightingData.ambientColor.toFloat4();
		sceneData.skyboxIndex = lightingData.skyboxIndex;

		// -- Primary Directional Light and Shadow Cascades --
		if (lightingData.hasDirectionalLight) {
			sceneData.hasDirectionalLight = 1;
			sceneData.directionalLight.direction = { lightingData.directionalLight.direction.x, lightingData.directionalLight.direction.y, lightingData.directionalLight.direction.z, 1.0f };
			sceneData.directionalLight.color = lightingData.directionalLight.color.toFloat4();

			calculateLightMatrices(camera, lightingData);

			uint32_t activeCascades = getActiveShadowCascades();
			sceneData.shadowParams = { (int)s_config.shadowQuality, (int)activeCascades, 0, 0 };

			if (s_config.shadowQuality == GraphicsQuality::Low) {
				sceneData.cascadeRadii = { 10.0f, 45.0f, 45.0f, 45.0f };
				sceneData.cascadeSplits = { 8.0f, 40.0f, 40.0f, 40.0f };
			}
			else if (s_config.shadowQuality == GraphicsQuality::Medium) {
				sceneData.cascadeRadii = { 7.0f, 22.0f, 70.0f, 70.0f };
				sceneData.cascadeSplits = { 5.5f, 18.0f, 60.0f, 60.0f };
			}
			else {
				sceneData.cascadeRadii = { 5.0f, 15.0f, 45.0f, 120.0f };
				sceneData.cascadeSplits = { 4.0f, 12.0f, 38.0f, 100.0f };
			}

			sceneData.lightSpaceMatrix = s_lightSpaceMatrices[0].transposed().toXM();
			sceneData.shadowMapIndex = s_shadowMapTextures[0] ? s_shadowMapTextures[0]->getBindlessIndex() : 0;

			for (int i = 0; i < 4; ++i) {
				sceneData.lightSpaceMatrices[i] = s_lightSpaceMatrices[i].transposed().toXM();
				int bindIdx = s_shadowMapTextures[i] ? s_shadowMapTextures[i]->getBindlessIndex() : 0;
				if (i == 0) sceneData.shadowMapIndices.x = bindIdx;
				if (i == 1) sceneData.shadowMapIndices.y = bindIdx;
				if (i == 2) sceneData.shadowMapIndices.z = bindIdx;
				if (i == 3) sceneData.shadowMapIndices.w = bindIdx;
			}
		}
		else {
			sceneData.hasDirectionalLight = 0;
			sceneData.directionalLight = {};
			sceneData.shadowParams = { 0, 0, 0, 0 };
			sceneData.lightSpaceMatrix = Mat4::identity().toXM();
			for (int i = 0; i < 4; i++) {
				s_lightSpaceMatrices[i] = Mat4::identity();
				sceneData.lightSpaceMatrices[i] = Mat4::identity().toXM();
			}
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
		SceneData sceneData{};
		Mat4 view = transform.inverse();
		Mat4 viewProj = view * projection;
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

	Ref<Texture2D> Renderer::getShadowMap(uint32_t cascadeIndex) {
		return s_shadowMapTextures[cascadeIndex];
	}

	const std::array<Mat4, 4>& Renderer::getLightSpaceMatrices() {
		return s_lightSpaceMatrices;
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

	RendererConfig& Renderer::getConfig() { return s_config; }
	void Renderer::setConfig(const RendererConfig& config) { s_config = config; }

	void Renderer::calculateLightMatrices(const Camera& camera, const LightingData& lightingData) {
		uint32_t activeCascades = getActiveShadowCascades();
		if (!lightingData.hasDirectionalLight || activeCascades == 0) {
			for (int i = 0; i < 4; i++) {
				s_lightSpaceMatrices[i] = Mat4::identity();
			}
			return;
		}

		Vec3 lightDir = lightingData.directionalLight.direction;
		float ldLen = std::sqrt(lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z);
		if (ldLen > 0.0001f) {
			lightDir = { lightDir.x / ldLen, lightDir.y / ldLen, lightDir.z / ldLen };
		}

		Mat4 camInv = camera.getViewMatrix().inverse();
		Vec3 camPos = { camInv.data()[12], camInv.data()[13], camInv.data()[14] };

		float lightDistance = 100.0f;
		Vec3 lightUp = (std::abs(lightDir.y) > 0.99f) ? Vec3(0.0f, 0.0f, 1.0f) : Vec3(0.0f, 1.0f, 0.0f);

		Mat4 baseLightView = Mat4::lookAt(lightDir * lightDistance, Vec3(0.0f, 0.0f, 0.0f), lightUp);
		Vec4 camPosLightSpace = baseLightView * Vec4(camPos.x, camPos.y, camPos.z, 1.0f);

		float cascadeRadii[4] = { 5.0f, 15.0f, 45.0f, 120.0f };
		if (s_config.shadowQuality == GraphicsQuality::Low) {
			cascadeRadii[0] = 10.0f; cascadeRadii[1] = 45.0f; cascadeRadii[2] = 45.0f; cascadeRadii[3] = 45.0f;
		}
		else if (s_config.shadowQuality == GraphicsQuality::Medium) {
			cascadeRadii[0] = 7.0f; cascadeRadii[1] = 22.0f; cascadeRadii[2] = 70.0f; cascadeRadii[3] = 70.0f;
		}

		float zNear = -100.0f;
		float zFar = 100.0f;

		for (uint32_t i = 0; i < activeCascades; ++i) {
			float orthoSize = cascadeRadii[i];
			float worldUnitsPerTexel = (orthoSize * 2.0f) / 2048.0f;

			float snappedX = std::floor(camPosLightSpace.x / worldUnitsPerTexel) * worldUnitsPerTexel;
			float snappedY = std::floor(camPosLightSpace.y / worldUnitsPerTexel) * worldUnitsPerTexel;

			Mat4 lightProj = Mat4::orthographicOffCenter(
				snappedX - orthoSize, snappedX + orthoSize,
				snappedY - orthoSize, snappedY + orthoSize,
				camPosLightSpace.z + zNear, camPosLightSpace.z + zFar
			);
			s_lightSpaceMatrices[i] = baseLightView * lightProj;
		}
	}

	uint32_t Renderer::getActiveShadowCascades() {
		switch (s_config.shadowQuality) {
			case GraphicsQuality::Off:		return 0;
			case GraphicsQuality::Low:		return 2;
			case GraphicsQuality::Medium:	return 3;
			case GraphicsQuality::High:		return 4;
			case GraphicsQuality::Ultra:	return 4;
		}
		return 4;
	}

}
