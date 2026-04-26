#include "axpch.h"
#include "Renderer3D.h"

#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/render/RenderCommand.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/EngineAssets.h"

namespace Axion {

	struct GPUInstanceData {
		DirectX::XMFLOAT4X4 modelMatrix;
		DirectX::XMFLOAT4 color;
		uint32_t boneOffset;
		uint32_t padding[3];
	};

	// -- Static Mesh --
	static Ref<VertexBuffer> s_instanceVertexBuffer;
	constexpr uint32_t MAX_INSTANCES = 10000;

	// -- Skeletal Mesh --
	static Ref<StructuredBuffer> s_skeletalInstanceBuffer;
	static Ref<StructuredBuffer> s_boneBuffer;
	static uint32_t s_currentBoneElementIndex = 0;
	constexpr uint32_t MAX_SKELETAL_INSTANCES = 1000;
	constexpr uint32_t MAX_BONES = MAX_SKELETAL_INSTANCES * 1000;

	void Renderer3D::initialize() {
		s_instanceVertexBuffer = VertexBuffer::createDynamic(MAX_INSTANCES * sizeof(ObjectBuffer), sizeof(ObjectBuffer));
		s_instanceVertexBuffer->setLayout({
			{ "COLOR", ShaderDataType::Float4, false, true },
			{ "ROW", ShaderDataType::Float4, false, true },
			{ "ROW", ShaderDataType::Float4, false, true },
			{ "ROW", ShaderDataType::Float4, false, true },
			{ "ROW", ShaderDataType::Float4, false, true }
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

	void Renderer3D::endScene() {
		// does nothing for now
	}

	void Renderer3D::beginFrame() {
		if (s_instanceVertexBuffer) s_instanceVertexBuffer->resetOffset();
		if (s_skeletalInstanceBuffer) s_skeletalInstanceBuffer->resetOffset();
		if (s_boneBuffer) s_boneBuffer->resetOffset();

		s_currentBoneElementIndex = 0;
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

		mesh->getVertexBuffer()->bind();
		mesh->getIndexBuffer()->bind();
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

	void Renderer3D::drawSkeletalMeshInstanced(Ref<SkeletalMesh>& mesh, uint32_t submeshIndex, Ref<Material>& material, const std::vector<SkeletalObjectBuffer>& instanceData) {
		if (instanceData.empty() || !material || !material->isValid()) return;

		material->setSkeletal(true);
		material->bind();
		Renderer::getSceneDataBuffer()->bind(0, Renderer::getSceneDataOffset());

		std::vector<GPUInstanceData> gpuInstances(instanceData.size());
		std::vector<DirectX::XMFLOAT4X4> flatBones;
		flatBones.reserve(instanceData.size() * 100);

		for (size_t i = 0; i < instanceData.size(); ++i) {
			gpuInstances[i].modelMatrix = instanceData[i].modelMatrix;
			gpuInstances[i].color = instanceData[i].color;
			gpuInstances[i].boneOffset = s_currentBoneElementIndex + (static_cast<uint32_t>(i) * 100);

			for (int b = 0; b < 100; ++b) {
				flatBones.push_back(instanceData[i].boneTransforms[b]);
			}
		}

		s_currentBoneElementIndex += static_cast<uint32_t>(flatBones.size());

		uint32_t instanceByteOffset = s_skeletalInstanceBuffer->append(gpuInstances.data(), gpuInstances.size() * sizeof(GPUInstanceData));
		uint32_t boneByteOffset = s_boneBuffer->append(flatBones.data(), flatBones.size() * sizeof(DirectX::XMFLOAT4X4));

		mesh->getVertexBuffer()->bind();
		mesh->getIndexBuffer()->bind();

		Ref<Pipeline> pipeline = AssetManager::get<Pipeline>(material->getPipelineHandle());
		if (!pipeline) { pipeline = EngineAssets::getSkeletalPBRPipeline(); }
		Ref<Shader> shader = pipeline->getSpecification().shader;

		uint32_t instanceSlot = shader->getBindPoint("u_instanceData");
		uint32_t boneSlot = shader->getBindPoint("u_boneData");

		s_skeletalInstanceBuffer->bind(instanceSlot, instanceByteOffset);
		s_boneBuffer->bind(boneSlot, 0);

		const auto& submeshes = mesh->getSubmeshes();
		auto& stats = Renderer::getStats();

		if (submeshes.empty()) {
			RenderCommand::drawIndexed(mesh->getIndexBuffer(), mesh->getIndexCount(), static_cast<uint32_t>(instanceData.size()), 0, 0);
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

		mesh->getVertexBuffer()->bind();
		mesh->getIndexBuffer()->bind();
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

	void Renderer3D::drawSkeletalMeshInstancedShadow(Ref<SkeletalMesh>& mesh, uint32_t submeshIndex, const std::vector<SkeletalObjectBuffer>& instanceData) {
		if (instanceData.empty()) return;

		Ref<Pipeline> shadowPipeline = EngineAssets::getSkeletalShadowPipeline();
		if (!shadowPipeline) return;

		shadowPipeline->bind();
		Renderer::getSceneDataBuffer()->bind(0, Renderer::getSceneDataOffset());

		std::vector<GPUInstanceData> gpuInstances(instanceData.size());
		std::vector<DirectX::XMFLOAT4X4> flatBones;
		flatBones.reserve(instanceData.size() * 100);

		for (size_t i = 0; i < instanceData.size(); ++i) {
			gpuInstances[i].modelMatrix = instanceData[i].modelMatrix;
			gpuInstances[i].color = instanceData[i].color;
			gpuInstances[i].boneOffset = s_currentBoneElementIndex + (static_cast<uint32_t>(i) * 100);

			for (int b = 0; b < 100; ++b) {
				flatBones.push_back(instanceData[i].boneTransforms[b]);
			}
		}

		s_currentBoneElementIndex += static_cast<uint32_t>(flatBones.size());

		uint32_t instanceByteOffset = s_skeletalInstanceBuffer->append(gpuInstances.data(), gpuInstances.size() * sizeof(GPUInstanceData));
		uint32_t boneByteOffset = s_boneBuffer->append(flatBones.data(), flatBones.size() * sizeof(DirectX::XMFLOAT4X4));

		mesh->getVertexBuffer()->bind();
		mesh->getIndexBuffer()->bind();

		Ref<Shader> shader = shadowPipeline->getSpecification().shader;
		uint32_t instanceSlot = shader->getBindPoint("u_instanceData");
		uint32_t boneSlot = shader->getBindPoint("u_boneData");

		s_skeletalInstanceBuffer->bind(instanceSlot, instanceByteOffset);
		s_boneBuffer->bind(boneSlot, 0);

		const auto& submeshes = mesh->getSubmeshes();
		auto& stats = Renderer::getStats();

		if (submeshes.empty()) {
			RenderCommand::drawIndexed(mesh->getIndexBuffer(), mesh->getIndexCount(), static_cast<uint32_t>(instanceData.size()), 0, 0);
		}
		else {
			const auto& submesh = submeshes[submeshIndex];
			RenderCommand::drawIndexed(mesh->getIndexBuffer(), submesh.indexCount, static_cast<uint32_t>(instanceData.size()), submesh.startIndex, submesh.baseVertex);
		}

		stats.drawCalls++;
		stats.meshCount3D++;
		stats.instanceCount3D += static_cast<uint32_t>(instanceData.size());
	}

}
