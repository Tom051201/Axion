#include "axpch.h"
#include "Renderer3D.h"

#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/render/RenderCommand.h"

namespace Axion {

	static Ref<VertexBuffer> s_instanceVertexBuffer;
	static uint32_t s_instanceBufferOffset = 0;
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
		s_instanceBufferOffset = 0;
	}

	void Renderer3D::beginScene(const Mat4& projection, const Mat4& transform) {
		Renderer::beginScene(projection, transform);
		s_instanceBufferOffset = 0;
	}

	void Renderer3D::endScene() {
		// does nothing for now
	}

	void Renderer3D::setClearColor(const Vec4& color) {
		RenderCommand::setClearColor(color);
	}

	void Renderer3D::clear() {
		RenderCommand::clear();
	}

	void Renderer3D::drawMesh(const Mat4& transform, Ref<Mesh>& mesh, Ref<Material>& material, Ref<ConstantBuffer>& uploadBuffer) {
		material->bind();

		Renderer::getSceneDataBuffer()->bind(0);

		ObjectBuffer buffer;
		buffer.color = material->getAlbedoColor().toFloat4();
		buffer.modelMatrix = transform.transposed().toXM();

		uploadBuffer->update(&buffer, sizeof(ObjectBuffer));
		uploadBuffer->bind(1);

		mesh->render();
		RenderCommand::drawIndexed(mesh->getVertexBuffer(), mesh->getIndexBuffer());

		auto& stats = Renderer::getStats();
		stats.drawCalls++;
		stats.meshCount3D++;
		stats.instanceCount3D++;
	}

	void Renderer3D::drawMeshInstanced(Ref<Mesh>& mesh, Ref<Material>& material, const std::vector<ObjectBuffer>& instanceData) {
		if (instanceData.empty()) return;

		material->bind();
		Renderer::getSceneDataBuffer()->bind(0);

		// Dummy
		Renderer::getSceneDataBuffer()->bind(1);

		uint32_t dataSize = static_cast<uint32_t>(instanceData.size() * sizeof(ObjectBuffer));

		AX_CORE_ASSERT(s_instanceBufferOffset + dataSize <= s_instanceVertexBuffer->getSize(), "Instance buffer overflow!");

		s_instanceVertexBuffer->update(instanceData.data(), dataSize, s_instanceBufferOffset);

		mesh->render();
		s_instanceVertexBuffer->bind(1, s_instanceBufferOffset);

		RenderCommand::drawIndexed(mesh->getVertexBuffer(), mesh->getIndexBuffer(), static_cast<uint32_t>(instanceData.size()));

		s_instanceBufferOffset += dataSize;

		auto& stats = Renderer::getStats();
		stats.drawCalls++;
		stats.meshCount3D++;
		stats.instanceCount3D += static_cast<uint32_t>(instanceData.size());
	}

}
