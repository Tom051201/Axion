#include "axpch.h"
#include "Renderer3D.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/EngineAssets.h"
#include "AxionEngine/Source/graphics/Renderer.h"
#include "AxionEngine/Source/graphics/RenderCommand.h"

namespace Axion {

	struct GPUInstanceData {
		DirectX::XMFLOAT4X4 modelMatrix;
		DirectX::XMFLOAT4 color;
		uint32_t boneOffset;
		int entityID;
		uint32_t padding[2];
	};

	// -- Static Mesh --
	static Ref<VertexBuffer> s_instanceVertexBuffer;
	constexpr uint32_t MAX_INSTANCES = 10000;

	// -- Skeletal Mesh --
	static Ref<StructuredBuffer> s_skeletalInstanceBuffer;
	static Ref<StructuredBuffer> s_boneBuffer;
	static uint32_t s_currentBoneElementIndex = 0;
	static uint32_t s_currentStaticInstanceCount = 0;
	static uint32_t s_currentSkeletalInstanceCount = 0;

	constexpr uint32_t MAX_SKELETAL_INSTANCES = 1000;
	constexpr uint32_t MAX_BONES = MAX_SKELETAL_INSTANCES * 100;

	void Renderer3D::initialize() {
		s_instanceVertexBuffer = VertexBuffer::createDynamic(MAX_INSTANCES * sizeof(ObjectBuffer), sizeof(ObjectBuffer));
		s_instanceVertexBuffer->setLayout({
			{ "COLOR", ShaderDataType::Float4, false, true },
			{ "ROW", ShaderDataType::Float4, false, true },
			{ "ROW", ShaderDataType::Float4, false, true },
			{ "ROW", ShaderDataType::Float4, false, true },
			{ "ROW", ShaderDataType::Float4, false, true },
			{ "ENTITY_ID", ShaderDataType::Int, false, true },
			{ "PADDING", ShaderDataType::Float3, false, true }
		});

		s_skeletalInstanceBuffer = StructuredBuffer::create(sizeof(GPUInstanceData), MAX_SKELETAL_INSTANCES);
		s_boneBuffer = StructuredBuffer::create(sizeof(DirectX::XMFLOAT4X4), MAX_BONES);

		AX_CORE_LOG_TRACE("Renderer3D initialized");
	}

	void Renderer3D::shutdown() {
		s_instanceVertexBuffer->release();
		s_skeletalInstanceBuffer->release();
		s_boneBuffer->release();

		AX_CORE_LOG_TRACE("Renderer3D shutdown");
	}

	void Renderer3D::beginScene(const Camera& cam, const LightingData& lightData) {
		Renderer::beginScene(cam, lightData);
	}

	void Renderer3D::beginScene(const Mat4& projection, const Mat4& transform) {
		Renderer::beginScene(projection, transform);
	}

	void Renderer3D::endScene() {}

	void Renderer3D::beginFrame() {
		if (s_instanceVertexBuffer) s_instanceVertexBuffer->resetOffset();
		if (s_skeletalInstanceBuffer) s_skeletalInstanceBuffer->resetOffset();
		if (s_boneBuffer) s_boneBuffer->resetOffset();

		s_currentBoneElementIndex = 0;
		s_currentStaticInstanceCount = 0;
		s_currentSkeletalInstanceCount = 0;
	}

	void Renderer3D::setClearColor(const Vec4& color) {
		RenderCommand::setClearColor(color);
	}

	void Renderer3D::clear(RenderContext* renderContext) {
		RenderCommand::clear(renderContext);
	}

	void Renderer3D::drawMesh(RenderContext* renderContext, const Mat4& transform, Ref<Mesh>& mesh, uint32_t submeshIndex, Ref<Material>& material, Ref<ConstantBuffer>& uploadBuffer) {
		if (!material || !material->isValid()) return;

		ObjectBuffer buffer;
		buffer.color = material->getAlbedoColor().toFloat4();
		buffer.modelMatrix = transform.transposed().toXM();
		buffer.entityID = -1;

		drawMeshInstanced(renderContext, mesh, submeshIndex, material, &buffer, 1);
	}

	void Renderer3D::drawMeshInstanced(RenderContext* renderContext, Ref<Mesh>& mesh, uint32_t submeshIndex, Ref<Material>& material, const ObjectBuffer* instanceData, uint32_t instanceCount) {
		if (instanceCount == 0 || !mesh || !material || !material->isValid()) return;

		if (s_currentStaticInstanceCount + instanceCount > MAX_INSTANCES) {
			AX_CORE_LOG_WARN("Renderer3D: Exceeded MAX_INSTANCES!");
			return;
		}

		material->bind(renderContext);
		EngineAssets::getStandardPBRPipeline()->bind(renderContext); // TODO: maybe review this and maybe remove or improve this so that the passed pipeline does not get ignored
		Renderer::getSceneDataBuffer()->bind(renderContext, 0, Renderer::getSceneDataOffset());

		uint32_t dataSize = instanceCount * sizeof(ObjectBuffer);
		uint32_t bufferOffset = s_instanceVertexBuffer->append(instanceData, dataSize);
		s_currentStaticInstanceCount += instanceCount;

		mesh->getVertexBuffer()->bind(renderContext);
		mesh->getIndexBuffer()->bind(renderContext);
		s_instanceVertexBuffer->bind(renderContext, 1, bufferOffset);

		const auto& submeshes = mesh->getSubmeshes();
		auto& stats = Renderer::getStats();

		if (submeshes.empty()) {
			RenderCommand::drawIndexed(renderContext, mesh->getIndexBuffer(), mesh->getIndexCount(), instanceCount, 0, 0);
		}
		else {
			const auto& submesh = submeshes[submeshIndex];
			RenderCommand::drawIndexed(renderContext, mesh->getIndexBuffer(), submesh.indexCount, instanceCount, submesh.startIndex, submesh.baseVertex);
		}

		stats.drawCalls++;
		stats.meshCount3D++;
		stats.instanceCount3D += instanceCount;
	}

	void Renderer3D::drawSkeletalMeshInstanced(RenderContext* renderContext, Ref<SkeletalMesh>& mesh, uint32_t submeshIndex, Ref<Material>& material, const SkeletalObjectBuffer* instanceData, uint32_t instanceCount) {
		if (instanceCount == 0 || !mesh || !material || !material->isValid()) return;

		if (s_currentSkeletalInstanceCount + instanceCount > MAX_SKELETAL_INSTANCES || s_currentBoneElementIndex + (instanceCount * 100) > MAX_BONES) {
			AX_CORE_LOG_WARN("Renderer3D: Exceeded MAX_SKELETAL_INSTANCES or MAX_BONES!");
			return;
		}

		material->setSkeletal(true);
		material->bind(renderContext);

		Ref<Pipeline> pbrPipeline = EngineAssets::getSkeletalPBRPipeline(); // TODO: maybe review as above
		pbrPipeline->bind(renderContext); // TODO: maybe review as above

		Renderer::getSceneDataBuffer()->bind(0, Renderer::getSceneDataOffset());

		thread_local std::vector<GPUInstanceData> gpuInstances;
		thread_local std::vector<DirectX::XMFLOAT4X4> flatBones;

		gpuInstances.clear();
		flatBones.clear();
		gpuInstances.reserve(instanceCount);
		flatBones.reserve(instanceCount * 100);

		for (size_t i = 0; i < instanceCount; ++i) {
			GPUInstanceData gpuInst;
			gpuInst.modelMatrix = instanceData[i].modelMatrix;
			gpuInst.color = instanceData[i].color;
			gpuInst.boneOffset = s_currentBoneElementIndex + (static_cast<uint32_t>(i) * 100);
			gpuInst.entityID = instanceData[i].entityID;
			gpuInstances.push_back(gpuInst);

			for (int b = 0; b < 100; ++b) {
				flatBones.push_back(instanceData[i].boneTransforms[b]);
			}
		}

		s_currentBoneElementIndex += static_cast<uint32_t>(flatBones.size());
		s_currentSkeletalInstanceCount += instanceCount;

		uint32_t instanceByteOffset = s_skeletalInstanceBuffer->append(gpuInstances.data(), gpuInstances.size() * sizeof(GPUInstanceData));
		uint32_t boneByteOffset = s_boneBuffer->append(flatBones.data(), flatBones.size() * sizeof(DirectX::XMFLOAT4X4));

		mesh->getVertexBuffer()->bind(renderContext);
		mesh->getIndexBuffer()->bind(renderContext);

		Ref<Pipeline> pipeline = AssetManager::get<Pipeline>(material->getPipelineHandle());
		if (!pipeline) { pipeline = EngineAssets::getSkeletalPBRPipeline(); }
		Ref<Shader> shader = pipeline->getSpecification().shader;

		uint32_t instanceSlot = shader->getBindPoint("u_instanceData");
		uint32_t boneSlot = shader->getBindPoint("u_boneData");

		s_skeletalInstanceBuffer->bind(renderContext, instanceSlot, instanceByteOffset);
		s_boneBuffer->bind(renderContext, boneSlot, 0);

		const auto& submeshes = mesh->getSubmeshes();
		auto& stats = Renderer::getStats();

		if (submeshes.empty()) {
			RenderCommand::drawIndexed(renderContext, mesh->getIndexBuffer(), mesh->getIndexCount(), instanceCount, 0, 0);
		}
		else {
			const auto& submesh = submeshes[submeshIndex];
			RenderCommand::drawIndexed(renderContext, mesh->getIndexBuffer(), submesh.indexCount, instanceCount, submesh.startIndex, submesh.baseVertex);
		}

		stats.drawCalls++;
		stats.meshCount3D++;
		stats.instanceCount3D += instanceCount;
	}

	void Renderer3D::drawMeshInstancedShadow(RenderContext* renderContext, Ref<Mesh>& mesh, uint32_t submeshIndex, const ObjectBuffer* instanceData, uint32_t instanceCount) {
		if (instanceCount == 0 || !mesh) return;

		if (s_currentStaticInstanceCount + instanceCount > MAX_INSTANCES) return;

		Ref<Pipeline> shadowPipeline = EngineAssets::getShadowPipeline();
		if (!shadowPipeline) return;

		shadowPipeline->bind(renderContext);
		Renderer::getSceneDataBuffer()->bind(renderContext, 0, Renderer::getSceneDataOffset());

		uint32_t dataSize = instanceCount * sizeof(ObjectBuffer);
		uint32_t bufferOffset = s_instanceVertexBuffer->append(instanceData, dataSize);
		s_currentStaticInstanceCount += instanceCount;

		mesh->getVertexBuffer()->bind(renderContext);
		mesh->getIndexBuffer()->bind(renderContext);
		s_instanceVertexBuffer->bind(renderContext, 1, bufferOffset);

		const auto& submeshes = mesh->getSubmeshes();

		if (submeshes.empty()) {
			RenderCommand::drawIndexed(renderContext, mesh->getIndexBuffer(), mesh->getIndexCount(), instanceCount, 0, 0);
		}
		else {
			const auto& submesh = submeshes[submeshIndex];
			RenderCommand::drawIndexed(renderContext, mesh->getIndexBuffer(), submesh.indexCount, instanceCount, submesh.startIndex, submesh.baseVertex);
		}

		auto& stats = Renderer::getStats();
		stats.drawCalls++;
		stats.meshCount3D++;
		stats.instanceCount3D += instanceCount;
	}

	void Renderer3D::drawSkeletalMeshInstancedShadow(RenderContext* renderContext, Ref<SkeletalMesh>& mesh, uint32_t submeshIndex, const SkeletalObjectBuffer* instanceData, uint32_t instanceCount) {
		if (instanceCount == 0 || !mesh) return;

		if (s_currentSkeletalInstanceCount + instanceCount > MAX_SKELETAL_INSTANCES || s_currentBoneElementIndex + (instanceCount * 100) > MAX_BONES) return;

		Ref<Pipeline> shadowPipeline = EngineAssets::getSkeletalShadowPipeline();
		if (!shadowPipeline) return;

		shadowPipeline->bind(renderContext);
		Renderer::getSceneDataBuffer()->bind(renderContext, 0, Renderer::getSceneDataOffset());

		thread_local std::vector<GPUInstanceData> gpuInstances;
		thread_local std::vector<DirectX::XMFLOAT4X4> flatBones;

		gpuInstances.clear();
		flatBones.clear();
		gpuInstances.reserve(instanceCount);
		flatBones.reserve(instanceCount * 100);

		for (size_t i = 0; i < instanceCount; ++i) {
			GPUInstanceData gpuInst;
			gpuInst.modelMatrix = instanceData[i].modelMatrix;
			gpuInst.color = instanceData[i].color;
			gpuInst.boneOffset = s_currentBoneElementIndex + (static_cast<uint32_t>(i) * 100);
			gpuInst.entityID = instanceData[i].entityID;
			gpuInstances.push_back(gpuInst);

			for (int b = 0; b < 100; ++b) {
				flatBones.push_back(instanceData[i].boneTransforms[b]);
			}
		}

		s_currentBoneElementIndex += static_cast<uint32_t>(flatBones.size());
		s_currentSkeletalInstanceCount += instanceCount;

		uint32_t instanceByteOffset = s_skeletalInstanceBuffer->append(gpuInstances.data(), gpuInstances.size() * sizeof(GPUInstanceData));
		uint32_t boneByteOffset = s_boneBuffer->append(flatBones.data(), flatBones.size() * sizeof(DirectX::XMFLOAT4X4));

		mesh->getVertexBuffer()->bind(renderContext);
		mesh->getIndexBuffer()->bind(renderContext);

		Ref<Shader> shader = shadowPipeline->getSpecification().shader;
		uint32_t instanceSlot = shader->getBindPoint("u_instanceData");
		uint32_t boneSlot = shader->getBindPoint("u_boneData");

		s_skeletalInstanceBuffer->bind(renderContext, instanceSlot, instanceByteOffset);
		s_boneBuffer->bind(renderContext, boneSlot, 0);

		const auto& submeshes = mesh->getSubmeshes();

		if (submeshes.empty()) {
			RenderCommand::drawIndexed(renderContext, mesh->getIndexBuffer(), mesh->getIndexCount(), instanceCount, 0, 0);
		}
		else {
			const auto& submesh = submeshes[submeshIndex];
			RenderCommand::drawIndexed(renderContext, mesh->getIndexBuffer(), submesh.indexCount, instanceCount, submesh.startIndex, submesh.baseVertex);
		}

		auto& stats = Renderer::getStats();
		stats.drawCalls++;
		stats.meshCount3D++;
		stats.instanceCount3D += instanceCount;
	}

}
