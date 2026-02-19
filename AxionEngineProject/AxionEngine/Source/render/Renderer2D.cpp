#include "axpch.h"
#include "Renderer2D.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/render/RenderCommand.h"
#include "AxionEngine/Source/render/Texture.h"

#include "AxionEngine/Platform/directx/D12Context.h"
#include "AxionEngine/Platform/directx/D12CommandList.h"

namespace Axion {

	struct QuadVertex {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 texCoord;
		float texIndex;
		float tilingFactor;
	};

	struct Renderer2DData {
		static const uint32_t MaxQuads = 20000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 16; // must match d12 bindSrvTable size, maybe add define

		Ref<VertexBuffer> quadVertexBuffer;
		Ref<IndexBuffer> quadIndexBuffer;
		Ref<Material> quadMaterial;

		uint32_t quadIndexCount = 0;
		QuadVertex* quadVertexBufferBase = nullptr;
		QuadVertex* quadVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> textureSlots;
		uint32_t textureSlotIndex = 1; // 0 = white fallback

		Vec4 quadVertexPositions[4];
		Vec2 quadTexCoords[4];

		struct CameraData {
			DirectX::XMFLOAT4X4 viewProjection;
		};
		Ref<ConstantBuffer> cameraConstantBuffer;

		Renderer2D::Statistics stats;
	};

	static Renderer2DData s_data;
	static bool s_initialized = false;

	void Renderer2D::initialize() {

		// -- Create dynamic VertexBuffer --
		s_data.quadVertexBuffer = VertexBuffer::createDynamic(s_data.MaxVertices * sizeof(QuadVertex), sizeof(QuadVertex));
		s_data.quadVertexBuffer->setLayout({
			{ "POSITION",	ShaderDataType::Float3, },
			{ "COLOR",		ShaderDataType::Float4, },
			{ "TEXCOORD",	ShaderDataType::Float2, },
			{ "TEXINDEX",	ShaderDataType::Float,  },
			{ "TILING",		ShaderDataType::Float,  }
		});


		// -- Allocate CPU-side memory for vertices --
		s_data.quadVertexBufferBase = new QuadVertex[s_data.MaxVertices];
		s_data.quadVertexBufferPtr = s_data.quadVertexBufferBase;


		// -- Create indices --
		uint32_t* quadIndices = new uint32_t[s_data.MaxIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_data.MaxIndices; i += 6) {
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}


		// -- Create static IndexBuffer --
		s_data.quadIndexBuffer = IndexBuffer::create(std::vector<uint32_t>(quadIndices, quadIndices + s_data.MaxIndices));
		delete[] quadIndices;


		// -- Create camera ConstantBuffer --
		s_data.cameraConstantBuffer = ConstantBuffer::create(sizeof(Renderer2DData::CameraData));


		// -- Setup quad positions --
		s_data.quadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };	// BL
		s_data.quadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };	// BR
		s_data.quadVertexPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };	// TR
		s_data.quadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };	// TL


		// -- Setup quad tex coords --
		s_data.quadTexCoords[0] = { 0.0f, 1.0f };	// BL
		s_data.quadTexCoords[1] = { 1.0f, 1.0f };	// BR
		s_data.quadTexCoords[2] = { 1.0f, 0.0f };	// TR
		s_data.quadTexCoords[3] = { 0.0f, 0.0f };	// TL


		// -- Slot 0 = white fallback --
		s_data.textureSlots[0] = Renderer::getWhiteFallbackTexture();


		s_initialized = true;
		AX_CORE_LOG_TRACE("Renderer2D initialized");
	}

	void Renderer2D::shutdown() {
		delete[] s_data.quadVertexBufferBase;
		s_data.quadVertexBuffer->release();
		s_data.quadIndexBuffer->release();
		s_data.cameraConstantBuffer->release();
		if (s_data.quadMaterial) s_data.quadMaterial->release();

		AX_CORE_LOG_TRACE("Renderer2D shutdown");
	}

	void Renderer2D::onEvent(Event& e) {
		if (e.getEventType() == EventType::ProjectChanged) {
			AssetHandle<Shader> shaderHandle = AssetManager::load<Shader>(AssetManager::getAbsolute("shaders/Batch2dShader.axshader"));
			s_data.quadMaterial = Material::create("Batch2DMat",/*{1.0f, 0.0f, 0.0f, 1.0f},*/ shaderHandle);
		}
	}

	void Renderer2D::beginScene(const Camera& camera) {
		//Renderer::beginScene(camera);
		Renderer2DData::CameraData camData;
		camData.viewProjection = camera.getViewProjectionMatrix().transposed().toFloat4x4();
		s_data.cameraConstantBuffer->update(&camData, sizeof(Renderer2DData::CameraData));
		startBatch();
	}

	void Renderer2D::endScene() {
		flush();
		resetStats();
	}

	void Renderer2D::startBatch() {
		s_data.quadIndexCount = 0;
		s_data.quadVertexBufferPtr = s_data.quadVertexBufferBase;
		s_data.textureSlotIndex = 1;
	}

	void Renderer2D::nextBatch() {
		flush();
		startBatch();
	}

	void Renderer2D::flush() {
		if (s_data.quadIndexCount == 0) return;

		uint32_t dataSize = (uint32_t)((uint8_t*)s_data.quadVertexBufferPtr - (uint8_t*)s_data.quadVertexBufferBase);
		s_data.quadVertexBuffer->update(s_data.quadVertexBufferBase, dataSize);

		s_data.quadMaterial->bind();
		s_data.cameraConstantBuffer->bind(0);

		Renderer::bindTextures(s_data.textureSlots, s_data.textureSlotIndex, 2);

		s_data.quadVertexBuffer->bind();
		s_data.quadIndexBuffer->bind();

		RenderCommand::drawIndexed(s_data.quadIndexBuffer, s_data.quadIndexCount);

		s_data.stats.drawCalls++;
	}

	void Renderer2D::drawQuad(const Vec2& position, const Vec2& size, const Vec4& color) {
		drawQuad({ position.x, position.y, 0.0f }, size, 0.0f, color);
	}

	void Renderer2D::drawQuad(const Vec3& position, const Vec2& size, const Vec4& color) {
		drawQuad(position, size, 0.0f, color);
	}

	void Renderer2D::drawQuad(const Vec2& position, const Vec2& size, float rotation, const Vec4& color) {
		drawQuad({ position.x, position.y, 0.0f }, size, rotation, color);
	}

	void Renderer2D::drawQuad(const Vec3& position, const Vec2& size, float rotation, const Vec4& color) {
		if (s_initialized) {
			// -- Check overflow --
			if (s_data.quadIndexCount >= Renderer2DData::MaxIndices) {
				nextBatch();
			}

			Mat4 transform = Mat4::TRS(position, { 0.0f, 0.0f, rotation }, { size.x, size.y, 1.0f });

			for (size_t i = 0; i < 4; i++) {
				Vec3 pos = (transform * s_data.quadVertexPositions[i]).xyz();

				s_data.quadVertexBufferPtr->position = { pos.x, pos.y, pos.z };
				s_data.quadVertexBufferPtr->color = { color.x, color.y, color.z, color.w };
				s_data.quadVertexBufferPtr->texCoord = { 0.0f, 0.0f };
				s_data.quadVertexBufferPtr->texIndex = 0.0f;
				s_data.quadVertexBufferPtr->tilingFactor = 1.0f;

				s_data.quadVertexBufferPtr++;
			}

			s_data.quadIndexCount += 6;
			s_data.stats.quadCount++;
		}
	}

	void Renderer2D::drawQuad(const Vec2& position, const Vec2& size, const Ref<Texture2D>& texture, const Vec4& tint) {
		drawQuad({ position.x, position.y, 0.0f }, size, 0.0f, texture, tint);
	}

	void Renderer2D::drawQuad(const Vec3& position, const Vec2& size, float rotation, const Ref<Texture2D>& texture, const Vec4& tint) {
		if (!s_initialized) return;

		if (s_data.quadIndexCount >= Renderer2DData::MaxIndices || s_data.textureSlotIndex >= Renderer2DData::MaxTextureSlots) {
			nextBatch();
		}

		float texIndex = -1.0f;

		// check if already in a slot
		for (uint32_t i = 0; i < s_data.textureSlotIndex; i++) {
			if (s_data.textureSlots[i]->getHandle() == texture->getHandle()) {
				texIndex = static_cast<float>(i);
				break;
			}
		}

		if (texIndex == -1) {
			texIndex = static_cast<float>(s_data.textureSlotIndex);
			s_data.textureSlots[s_data.textureSlotIndex] = texture;
			s_data.textureSlotIndex++;
		}

		Mat4 transform = Mat4::TRS(position, { 0.0f, 0.0f, rotation }, { size.x, size.y, 1.0f });

		for (size_t i = 0; i < 4; i++) {
			Vec3 pos = (transform * s_data.quadVertexPositions[i]).xyz();

			s_data.quadVertexBufferPtr->position = { pos.x, pos.y, pos.z };
			s_data.quadVertexBufferPtr->color = { tint.x, tint.y, tint.z, tint.w };
			s_data.quadVertexBufferPtr->texCoord = { s_data.quadTexCoords[i].x, s_data.quadTexCoords[i].y };
			s_data.quadVertexBufferPtr->texIndex = texIndex;
			s_data.quadVertexBufferPtr->tilingFactor = 1.0f;

			s_data.quadVertexBufferPtr++;
		}

		s_data.quadIndexCount += 6;
		s_data.stats.quadCount++;

	}


	Renderer2D::Statistics Renderer2D::getStats() {
		return s_data.stats;
	}

	void Renderer2D::resetStats() {
		memset(&s_data.stats, 0, sizeof(Statistics));
	}

}
