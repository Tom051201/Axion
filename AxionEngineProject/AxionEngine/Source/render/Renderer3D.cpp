#include "axpch.h"
#include "Renderer3D.h"

#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/render/RenderCommand.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/EngineAssets.h"

namespace Axion {

	static Ref<VertexBuffer> s_instanceVertexBuffer;
	constexpr uint32_t MAX_INSTANCES = 10000;

	void Renderer3D::initialize() {
		s_instanceVertexBuffer = VertexBuffer::createDynamic(MAX_INSTANCES * sizeof(ObjectBuffer), sizeof(ObjectBuffer));
		s_instanceVertexBuffer->setLayout({
			{ "COLOR", ShaderDataType::Float4, false, true },
			{ "ROW", ShaderDataType::Float4, false, true },
			{ "ROW", ShaderDataType::Float4, false, true },
			{ "ROW", ShaderDataType::Float4, false, true },
			{ "ROW", ShaderDataType::Float4, false, true }
		});

		AX_CORE_LOG_TRACE("Renderer3D initialized");
	}

	void Renderer3D::shutdown() {
		s_instanceVertexBuffer->release();

		AX_CORE_LOG_TRACE("Renderer3D shutdown");
	}

	void Renderer3D::beginScene(const Camera& cam, const LightingData& lightData) {
		Renderer::beginScene(cam, lightData);
	}

	void Renderer3D::beginScene(const Mat4& projection, const Mat4& transform) {
		Renderer::beginScene(projection, transform);
	}

	void Renderer3D::endScene() {
		// does nothing for now
	}

	void Renderer3D::beginFrame() {
		if (s_instanceVertexBuffer) {
			s_instanceVertexBuffer->resetOffset();
		}
	}

	void Renderer3D::setClearColor(const Vec4& color) {
		RenderCommand::setClearColor(color);
	}

	void Renderer3D::clear() {
		RenderCommand::clear();
	}

	void Renderer3D::drawMesh(const Mat4& transform, Ref<Mesh>& mesh, uint32_t submeshIndex, Ref<Material>& material, Ref<ConstantBuffer>& uploadBuffer) {
		if (!material || !material->isValid()) return;

		std::vector<ObjectBuffer> singleInstance;
		ObjectBuffer buffer;
		buffer.color = material->getAlbedoColor().toFloat4();
		buffer.modelMatrix = transform.transposed().toXM();
		singleInstance.push_back(buffer);

		drawMeshInstanced(mesh, submeshIndex, material, singleInstance);
	}

	void Renderer3D::drawMeshInstanced(Ref<Mesh>& mesh, uint32_t submeshIndex, Ref<Material>& material, const std::vector<ObjectBuffer>& instanceData) {
		if (instanceData.empty()) return;
		if (!material || !material->isValid()) return;

		material->bind();
		Renderer::getSceneDataBuffer()->bind(0, Renderer::getSceneDataOffset());

		uint32_t dataSize = static_cast<uint32_t>(instanceData.size() * sizeof(ObjectBuffer));
		uint32_t bufferOffset = s_instanceVertexBuffer->append(instanceData.data(), dataSize);

		mesh->render();
		s_instanceVertexBuffer->bind(1, bufferOffset);

		const auto& submeshes = mesh->getSubmeshes();
		auto& stats = Renderer::getStats();

		if (submeshes.empty()) {
			RenderCommand::drawIndexed(mesh->getVertexBuffer(), mesh->getIndexBuffer(), static_cast<uint32_t>(instanceData.size()));
		}
		else {
			const auto& submesh = submeshes[submeshIndex];
			RenderCommand::drawIndexed(mesh->getIndexBuffer(), submesh.indexCount, static_cast<uint32_t>(instanceData.size()), submesh.startIndex, submesh.baseVertex);
		}

		stats.drawCalls++;
		stats.meshCount3D++;
		stats.instanceCount3D += static_cast<uint32_t>(instanceData.size());
	}

	void Renderer3D::drawMeshInstancedShadow(Ref<Mesh>& mesh, uint32_t submeshIndex, const std::vector<ObjectBuffer>& instanceData) {
		if (instanceData.empty()) return;
		Ref<Pipeline> shadowPipeline = EngineAssets::getShadowPipeline();
		if (!shadowPipeline) return;

		shadowPipeline->bind();
		Renderer::getSceneDataBuffer()->bind(0, Renderer::getSceneDataOffset());

		uint32_t dataSize = static_cast<uint32_t>(instanceData.size() * sizeof(ObjectBuffer));
		uint32_t bufferOffset = s_instanceVertexBuffer->append(instanceData.data(), dataSize);

		mesh->render();
		s_instanceVertexBuffer->bind(1, bufferOffset);

		const auto& submeshes = mesh->getSubmeshes();

		if (submeshes.empty()) {
			RenderCommand::drawIndexed(mesh->getIndexBuffer(), mesh->getIndexCount(), static_cast<uint32_t>(instanceData.size()), 0, 0);
		}
		else {
			const auto& submesh = submeshes[submeshIndex];
			RenderCommand::drawIndexed(mesh->getIndexBuffer(), submesh.indexCount, static_cast<uint32_t>(instanceData.size()), submesh.startIndex, submesh.baseVertex);
		}

	}

}
