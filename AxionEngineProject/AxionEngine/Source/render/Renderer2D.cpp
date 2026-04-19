#include "axpch.h"
#include "Renderer2D.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/EngineAssets.h"
#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/render/RenderCommand.h"
#include "AxionEngine/Source/render/Texture.h"

#include "AxionEngine/Platform/directx/DX12Context.h"
#include "AxionEngine/Platform/directx/DX12CommandList.h"

#include "AxionEngine/Resources/shaders/Batch2DQuad_VS.h"
#include "AxionEngine/Resources/shaders/Batch2DQuad_PS.h"
#include "AxionEngine/Resources/shaders/Batch2DLine_VS.h"
#include "AxionEngine/Resources/shaders/Batch2DLine_PS.h"

namespace Axion {

	struct LineVertex {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};

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
		Ref<Pipeline> quadPipeline;
		Ref<Shader> quadShader;

		uint32_t quadIndexCount = 0;
		QuadVertex* quadVertexBufferBase = nullptr;
		QuadVertex* quadVertexBufferPtr = nullptr;

		static const uint32_t MaxLines = 10000;
		static const uint32_t MaxLineVertices = MaxLines * 2;

		Ref<VertexBuffer> lineVertexBuffer;
		Ref<Pipeline> linePipeline;
		Ref<Shader> lineShader;

		uint32_t lineVertexCount = 0;
		LineVertex* lineVertexBufferBase = nullptr;
		LineVertex* lineVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> textureSlots;
		uint32_t textureSlotIndex = 1; // 0 = white fallback

		Vec4 quadVertexPositions[4];
		Vec2 quadTexCoords[4];

		struct CameraData {
			DirectX::XMFLOAT4X4 viewProjection;
		};
		Ref<ConstantBuffer> cameraConstantBuffer;
		uint32_t cameraBufferOffset = 0;

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


		s_data.lineVertexBuffer = VertexBuffer::createDynamic(s_data.MaxLineVertices * sizeof(LineVertex), sizeof(LineVertex));
		s_data.lineVertexBuffer->setLayout({
			{ "POSITION",	ShaderDataType::Float3},
			{ "COLOR",		ShaderDataType::Float4 }
		});

		s_data.lineVertexBufferBase = new LineVertex[s_data.MaxLineVertices];
		s_data.lineVertexBufferPtr = s_data.lineVertexBufferBase;

		// -- Setup Quad Shader and Pipeline --
		ShaderSpecification qsSpec;
		qsSpec.name = "Batch2DQuad";
		qsSpec.batchTextures = Renderer2DData::MaxTextureSlots;
		s_data.quadShader = Shader::create(qsSpec);
		s_data.quadShader->loadFromBytecode(
			g_Batch2DQuad_VS, sizeof(g_Batch2DQuad_VS),
			g_Batch2DQuad_PS, sizeof(g_Batch2DQuad_PS)
		);
		PipelineSpecification qpSpec;
		qpSpec.shader = s_data.quadShader;
		qpSpec.numRenderTargets = 1;
		qpSpec.colorFormat = ColorFormat::RGBA8;
		qpSpec.depthStencilFormat = DepthStencilFormat::DEPTH32F;
		qpSpec.depthTest = true;
		qpSpec.depthWrite = false;
		qpSpec.depthFunction = DepthCompare::Less;
		qpSpec.stencilEnabled = false;
		qpSpec.sampleCount = 1;
		qpSpec.cullMode = CullMode::None;
		qpSpec.topology = PrimitiveTopology::TriangleList;
		qpSpec.vertexLayout = {
			{ "POSITION",	ShaderDataType::Float3 },
			{ "COLOR",		ShaderDataType::Float4 },
			{ "TEXCOORD",	ShaderDataType::Float2 },
			{ "TEXINDEX",	ShaderDataType::Float  },
			{ "TILING",		ShaderDataType::Float  }
		};
		s_data.quadPipeline = Pipeline::create(qpSpec);

		// -- Setup Line Shader and Pipeline --
		ShaderSpecification lsSpec;
		lsSpec.name = "Batch2DLine";
		lsSpec.batchTextures = 0;
		s_data.lineShader = Shader::create(lsSpec);
		s_data.lineShader->loadFromBytecode(
			g_Batch2DLine_VS, sizeof(g_Batch2DLine_VS),
			g_Batch2DLine_PS, sizeof(g_Batch2DLine_PS)
		);
		PipelineSpecification lpSpec;
		lpSpec.shader = s_data.lineShader;
		lpSpec.numRenderTargets = 1;
		lpSpec.colorFormat = ColorFormat::RGBA8;
		lpSpec.depthStencilFormat = DepthStencilFormat::DEPTH32F;
		lpSpec.depthTest = true;
		lpSpec.depthWrite = false;
		lpSpec.depthFunction = DepthCompare::Less;
		lpSpec.stencilEnabled = false;
		lpSpec.sampleCount = 1;
		lpSpec.cullMode = CullMode::None;
		lpSpec.topology = PrimitiveTopology::LineList;
		lpSpec.vertexLayout = {
			{ "POSITION",	ShaderDataType::Float3 },
			{ "COLOR",		ShaderDataType::Float4 }
		};
		s_data.linePipeline = Pipeline::create(lpSpec);

		// -- Create camera ConstantBuffer --
		const uint32_t MaxCameraPasses = 100;
		uint32_t alignedCameraDataSize = (sizeof(Renderer2DData::CameraData) + 255) & ~255;
		s_data.cameraConstantBuffer = ConstantBuffer::create(alignedCameraDataSize * MaxCameraPasses);


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
		s_data.textureSlots[0] = EngineAssets::getWhiteTexture();


		s_initialized = true;
		AX_CORE_LOG_TRACE("Renderer2D initialized");
	}

	void Renderer2D::shutdown() {
		delete[] s_data.quadVertexBufferBase;
		s_data.quadVertexBuffer->release();
		s_data.quadIndexBuffer->release();
		s_data.quadPipeline->release();
		s_data.quadShader->release();

		delete[] s_data.lineVertexBufferBase;
		s_data.lineVertexBuffer->release();
		s_data.linePipeline->release();
		s_data.lineShader->release();

		s_data.cameraConstantBuffer->release();

		for (uint32_t i = 0; i < Renderer2DData::MaxTextureSlots; i++) {
			s_data.textureSlots[i] = nullptr;
		}

		AX_CORE_LOG_TRACE("Renderer2D shutdown");
	}

	void Renderer2D::beginScene(const Camera& camera) {
		Renderer2DData::CameraData camData;
		camData.viewProjection = camera.getViewProjectionMatrix().transposed().toFloat4x4();
		s_data.cameraBufferOffset = s_data.cameraConstantBuffer->append(&camData, sizeof(Renderer2DData::CameraData));
		startBatch();
	}

	void Renderer2D::endScene() {
		flush();
	}

	void Renderer2D::beginFrame() {
		if (s_data.quadVertexBuffer) s_data.quadVertexBuffer->resetOffset();

		if (s_data.lineVertexBuffer) s_data.lineVertexBuffer->resetOffset();

		if (s_data.cameraConstantBuffer) s_data.cameraConstantBuffer->resetOffset();
	}

	void Renderer2D::startBatch() {
		s_data.quadIndexCount = 0;
		s_data.quadVertexBufferPtr = s_data.quadVertexBufferBase;
		s_data.textureSlotIndex = 1;

		s_data.lineVertexCount = 0;
		s_data.lineVertexBufferPtr = s_data.lineVertexBufferBase;
	}

	void Renderer2D::nextBatch() {
		flush();
		startBatch();
	}

	void Renderer2D::flush() {
		// -- Flush quads --
		if (s_data.quadIndexCount > 0) {
			Ref<Pipeline> pipeline = s_data.quadPipeline;
			if (!pipeline) return;

			uint32_t dataSize = (uint32_t)((uint8_t*)s_data.quadVertexBufferPtr - (uint8_t*)s_data.quadVertexBufferBase);
			uint32_t bufferOffset = s_data.quadVertexBuffer->append(s_data.quadVertexBufferBase, dataSize);

			pipeline->bind();
			s_data.cameraConstantBuffer->bind(0, s_data.cameraBufferOffset);
			Renderer::bindTextures(s_data.textureSlots, s_data.textureSlotIndex, 1);

			s_data.quadVertexBuffer->bind(0, bufferOffset);
			s_data.quadIndexBuffer->bind();

			RenderCommand::drawIndexed(s_data.quadIndexBuffer, s_data.quadIndexCount);

			Renderer::getStats().drawCalls++;
		}

		// -- Flush lines --
		if (s_data.lineVertexCount > 0) {
			Ref<Pipeline> pipeline = s_data.linePipeline;
			if (!pipeline) return;

			uint32_t dataSize = (uint32_t)((uint8_t*)s_data.lineVertexBufferPtr - (uint8_t*)s_data.lineVertexBufferBase);
			uint32_t bufferOffset = s_data.lineVertexBuffer->append(s_data.lineVertexBufferBase, dataSize);

			pipeline->bind();
			s_data.cameraConstantBuffer->bind(0, s_data.cameraBufferOffset);
			s_data.lineVertexBuffer->bind(0, bufferOffset);

			RenderCommand::draw(s_data.lineVertexCount);

			Renderer::getStats().drawCalls++;
		}
	}

	void Renderer2D::drawBillboard(const Vec3& position, const Vec2& size, const Mat4& cameraView, const Vec4& color) {
		if (!s_initialized) return;

		if (s_data.quadIndexCount >= Renderer2DData::MaxIndices) {
			nextBatch();
		}

		Mat4 camTransform = cameraView.inverse();

		Vec3 camRight = { camTransform.data()[0], camTransform.data()[1], camTransform.data()[2] };
		Vec3 camUp = { camTransform.data()[4], camTransform.data()[5], camTransform.data()[6] };

		camRight = camRight.normalized();
		camUp = camUp.normalized();

		Vec3 halfSize = { size.x * 0.5f, size.y * 0.5f, 0.0f };

		Vec3 vertices[4];
		vertices[0] = position - (camRight * halfSize.x) - (camUp * halfSize.y); // BL
		vertices[1] = position + (camRight * halfSize.x) - (camUp * halfSize.y); // BR
		vertices[2] = position + (camRight * halfSize.x) + (camUp * halfSize.y); // TR
		vertices[3] = position - (camRight * halfSize.x) + (camUp * halfSize.y); // TL

		for (size_t i = 0; i < 4; i++) {
			s_data.quadVertexBufferPtr->position = { vertices[i].x, vertices[i].y, vertices[i].z };
			s_data.quadVertexBufferPtr->color = { color.x, color.y, color.z, color.w };
			s_data.quadVertexBufferPtr->texCoord = { s_data.quadTexCoords[i].x, s_data.quadTexCoords[i].y };
			s_data.quadVertexBufferPtr->texIndex = 0.0f; // 0 = White Fallback
			s_data.quadVertexBufferPtr->tilingFactor = 1.0f;
			s_data.quadVertexBufferPtr++;
		}

		s_data.quadIndexCount += 6;
		Renderer::getStats().quadCount2D++;
	}

	void Renderer2D::drawBillboard(const Vec3& position, const Vec2& size, const Mat4& cameraView, const Ref<Texture2D>& texture, const Vec4& tint) {
		if (!s_initialized) return;

		if (s_data.quadIndexCount >= Renderer2DData::MaxIndices || s_data.textureSlotIndex >= Renderer2DData::MaxTextureSlots) {
			nextBatch();
		}

		float texIndex = -1.0f;

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

		Vec3 camRight = { cameraView.data()[0], cameraView.data()[4], cameraView.data()[8] };
		Vec3 camUp = { cameraView.data()[1], cameraView.data()[5], cameraView.data()[9] };

		camRight.normalized();
		camUp.normalized();

		Vec3 halfSize = { size.x * 0.5f, size.y * 0.5f, 0.0f };

		Vec3 vertices[4];
		vertices[0] = position - (camRight * halfSize.x) - (camUp * halfSize.y); // BL
		vertices[1] = position + (camRight * halfSize.x) - (camUp * halfSize.y); // BR
		vertices[2] = position + (camRight * halfSize.x) + (camUp * halfSize.y); // TR
		vertices[3] = position - (camRight * halfSize.x) + (camUp * halfSize.y); // TL

		for (size_t i = 0; i < 4; i++) {
			s_data.quadVertexBufferPtr->position = { vertices[i].x, vertices[i].y, vertices[i].z };
			s_data.quadVertexBufferPtr->color = { tint.x, tint.y, tint.z, tint.w };
			s_data.quadVertexBufferPtr->texCoord = { s_data.quadTexCoords[i].x, s_data.quadTexCoords[i].y };
			s_data.quadVertexBufferPtr->texIndex = texIndex;
			s_data.quadVertexBufferPtr->tilingFactor = 1.0f;
			s_data.quadVertexBufferPtr++;
		}

		s_data.quadIndexCount += 6;
		Renderer::getStats().quadCount2D++;
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
			Renderer::getStats().quadCount2D++;
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
		Renderer::getStats().quadCount2D++;
	}

	void Renderer2D::drawLine(const Vec3& p0, const Vec3& p1, const Vec4& color) {
		if (!s_initialized) return;

		if (s_data.lineVertexCount >= Renderer2DData::MaxLineVertices) {
			nextBatch();
		}

		s_data.lineVertexBufferPtr->position = { p0.x, p0.y, p0.z };
		s_data.lineVertexBufferPtr->color = { color.x, color.y, color.z, color.w };
		s_data.lineVertexBufferPtr++;

		s_data.lineVertexBufferPtr->position = { p1.x, p1.y, p1.z };
		s_data.lineVertexBufferPtr->color = { color.x, color.y, color.z, color.w };
		s_data.lineVertexBufferPtr++;

		s_data.lineVertexCount += 2;
		Renderer::getStats().lineCount2D++;
	}

}
