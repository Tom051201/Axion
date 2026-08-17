#include "axpch.h"
#include "BinaryAssetLoader.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/BinaryHeaders.h"
#include "AxionEngine/Source/core/JobSystem.h"
#include "AxionEngine/Source/graphics/Mesh.h"
#include "AxionEngine/Source/graphics/SkeletalMesh.h"
#include "AxionEngine/Source/graphics/Shader.h"
#include "AxionEngine/Source/graphics/Material.h"
#include "AxionEngine/Source/graphics/Renderer.h"
#include "AxionEngine/Source/graphics/Texture.h"
#include "AxionEngine/Source/scene/Skybox.h"
#include "AxionEngine/Source/scene/Prefab.h"
#include "AxionEngine/Source/scene/Animation.h"
#include "AxionEngine/Source/audio/AudioClip.h"
#include "AxionEngine/Source/physics/PhysicsMaterial.h"

namespace Axion {

	UUID BinaryAssetLoader::peekUUID(const std::filesystem::path& absolutePath) {
		std::ifstream in(absolutePath, std::ios::in | std::ios::binary);
		if (!in) return {};

		uint32_t type;
		UUID uuid;

		in.read(reinterpret_cast<char*>(&type), sizeof(uint32_t));
		in.read(reinterpret_cast<char*>(&uuid), sizeof(UUID));

		return uuid;
	}

	void BinaryAssetLoader::loadMesh(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<Mesh>().assets[handle] = nullptr;
		AssetManager::storage<Mesh>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- Background Thread --
			if (absolutePath.empty() || !std::filesystem::exists(absolutePath)) {
				AX_CORE_LOG_ERROR("Mesh binary missing: {}", absolutePath.string());
				return;
			}

			std::ifstream in(absolutePath, std::ios::in | std::ios::binary);

			BinaryAssetHeader header;
			in.read(reinterpret_cast<char*>(&header), sizeof(BinaryAssetHeader));

			uint32_t vertexCount = 0, indexCount = 0, submeshCount = 0;
			in.read(reinterpret_cast<char*>(&vertexCount), sizeof(uint32_t));
			in.read(reinterpret_cast<char*>(&indexCount), sizeof(uint32_t));

			if (header.version >= 2) {
				in.read(reinterpret_cast<char*>(&submeshCount), sizeof(uint32_t));
			}

			if (vertexCount > 1000000 || indexCount > 3000000) {
				AX_CORE_LOG_ERROR("Mesh Corrupted! Vertices: {}. Path: {}", vertexCount, absolutePath.string());
				return;
			}

			auto meshData = std::make_shared<MeshData>();
			meshData->vertices.resize(vertexCount);
			meshData->indices.resize(indexCount);

			in.read(reinterpret_cast<char*>(meshData->vertices.data()), vertexCount * sizeof(Vertex));
			in.read(reinterpret_cast<char*>(meshData->indices.data()), indexCount * sizeof(uint32_t));

			if (submeshCount > 0) {
				meshData->submeshes.resize(submeshCount);
				in.read(reinterpret_cast<char*>(meshData->submeshes.data()), submeshCount * sizeof(Submesh));
			}

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<Mesh>(handle, [meshData]() -> Ref<Mesh> {
				return Mesh::create(*meshData);
			});

		});
	}

	void BinaryAssetLoader::loadTexture2D(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<Texture2D>().assets[handle] = nullptr;
		AssetManager::storage<Texture2D>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- Background Thread --
			if (absolutePath.empty() || !std::filesystem::exists(absolutePath)) {
				AX_CORE_LOG_ERROR("Texture binary missing: {}", absolutePath.string());
				return;
			}

			std::ifstream in(absolutePath, std::ios::in | std::ios::binary);
			BinaryAssetHeader header;
			in.read(reinterpret_cast<char*>(&header), sizeof(BinaryAssetHeader));

			uint64_t dataSize = 0;
			in.read(reinterpret_cast<char*>(&dataSize), sizeof(uint64_t));

			if (dataSize == 0 || dataSize > 100000000) {
				AX_CORE_LOG_ERROR("Texture Corrupted or Missing! Path: {}", absolutePath.string());
				return;
			}

			auto imageData = std::make_shared<std::vector<uint8_t>>(dataSize);
			in.read(reinterpret_cast<char*>(imageData->data()), dataSize);

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<Texture2D>(handle, [imageData]() -> Ref<Texture2D> {
				return Texture2D::create(imageData->data(), imageData->size());
			});

		});
	}

	void BinaryAssetLoader::loadTextureCube(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<TextureCube>().assets[handle] = nullptr;
		AssetManager::storage<TextureCube>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- Background Thread --
			if (absolutePath.empty() || !std::filesystem::exists(absolutePath)) {
				AX_CORE_LOG_ERROR("TextureCube binary missing: {}", absolutePath.string());
				return;
			}

			std::ifstream in(absolutePath, std::ios::in | std::ios::binary);

			BinaryAssetHeader header;
			in.read(reinterpret_cast<char*>(&header), sizeof(BinaryAssetHeader));

			uint64_t dataSize;
			in.read(reinterpret_cast<char*>(&dataSize), sizeof(uint64_t));

			if (dataSize == 0) {
				AX_CORE_LOG_ERROR("TextureCube Corrupted or Missing! Path: {}", absolutePath.string());
				return;
			}

			auto imageData = std::make_shared<std::vector<uint8_t>>(dataSize);
			in.read(reinterpret_cast<char*>(imageData->data()), dataSize);

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<TextureCube>(handle, [imageData]() -> Ref<TextureCube> {
				return TextureCube::create(imageData->data(), imageData->size());
			});

		});
	}

	void BinaryAssetLoader::loadSkybox(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<Skybox>().assets[handle] = nullptr;
		AssetManager::storage<Skybox>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- Background Thread --
			if (absolutePath.empty() || !std::filesystem::exists(absolutePath)) {
				AX_CORE_LOG_ERROR("Skybox binary missing: {}", absolutePath.string());
				return;
			}

			std::ifstream in(absolutePath, std::ios::in | std::ios::binary);
			SkyboxBinaryHeader header;
			in.read(reinterpret_cast<char*>(&header), sizeof(SkyboxBinaryHeader));

			UUID texUUID = header.textureCubeUUID;
			UUID pipeUUID = header.pipelineUUID;

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<Skybox>(handle, [texUUID, pipeUUID]() -> Ref<Skybox> {
				AssetHandle<TextureCube> texHandle = AssetManager::load<TextureCube>(texUUID);

				if (pipeUUID.isValid()) {
					AssetHandle<Pipeline> pipeHandle = AssetManager::load<Pipeline>(pipeUUID);
					return std::make_shared<Skybox>(texHandle, pipeHandle);
				}
				else {
					return std::make_shared<Skybox>(texHandle);
				}
			});

		});
	}

	void BinaryAssetLoader::loadShader(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<Shader>().assets[handle] = nullptr;
		AssetManager::storage<Shader>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- Disk IO On Background Thread --
			std::ifstream in(absolutePath, std::ios::in | std::ios::binary);
			if (!in.is_open()) {
				AX_CORE_LOG_ERROR("Shader binary missing: {}", absolutePath.string());
				return;
			}

			ShaderBinaryHeader header;
			in.read(reinterpret_cast<char*>(&header), sizeof(ShaderBinaryHeader));

			ShaderSpecification spec = {};
			spec.name = "RuntimeShader_" + header.assetHeader.uuid.toString();
			spec.batchTextures = header.batchTextures;

			auto vsData = std::make_shared<std::vector<uint8_t>>(header.vsSize);
			in.read(reinterpret_cast<char*>(vsData->data()), header.vsSize);

			auto psData = std::make_shared<std::vector<uint8_t>>(header.psSize);
			in.read(reinterpret_cast<char*>(psData->data()), header.psSize);

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<Shader>(handle, [spec, vsData, psData]() -> Ref<Shader> {
				Ref<Shader> shader = Shader::create(spec);
				shader->loadFromBytecode(vsData->data(), vsData->size(), psData->data(), psData->size());
				return shader;
			});

		});
	}

	void BinaryAssetLoader::loadPipeline(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<Pipeline>().assets[handle] = nullptr;
		AssetManager::storage<Pipeline>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- Disk IO And Parsing On Background Thread --
			if (absolutePath.empty() || !std::filesystem::exists(absolutePath)) {
				AX_CORE_LOG_ERROR("Pipeline binary missing: {}", absolutePath.string());
				return;
			}

			std::ifstream in(absolutePath, std::ios::in | std::ios::binary);

			PipelineBinaryHeader header;
			in.read(reinterpret_cast<char*>(&header), sizeof(PipelineBinaryHeader));

			auto spec = std::make_shared<PipelineSpecification>();
			spec->colorFormat = static_cast<ColorFormat>(header.colorFormat);
			spec->depthStencilFormat = static_cast<DepthStencilFormat>(header.depthStencilFormat);
			spec->depthTest = header.depthTest != 0;
			spec->depthWrite = header.depthWrite != 0;
			spec->depthFunction = static_cast<DepthCompare>(header.depthFunction);
			spec->stencilEnabled = header.stencilEnabled != 0;
			spec->sampleCount = header.sampleCount;
			spec->cullMode = static_cast<CullMode>(header.cullMode);
			spec->topology = static_cast<PrimitiveTopology>(header.topology);
			spec->numRenderTargets = header.numRenderTargets;

			std::vector<BufferElement> elements;
			elements.reserve(header.bufferElementCount);

			for (uint32_t i = 0; i < header.bufferElementCount; i++) {
				uint32_t nameLen;
				in.read(reinterpret_cast<char*>(&nameLen), sizeof(uint32_t));
				std::string semanticName(nameLen, '\0');
				in.read(&semanticName[0], nameLen);

				uint32_t typeInt, size, offset;
				uint8_t normalized, instanced;

				in.read(reinterpret_cast<char*>(&typeInt), sizeof(uint32_t));
				in.read(reinterpret_cast<char*>(&size), sizeof(uint32_t));
				in.read(reinterpret_cast<char*>(&offset), sizeof(uint32_t));
				in.read(reinterpret_cast<char*>(&normalized), sizeof(uint8_t));
				in.read(reinterpret_cast<char*>(&instanced), sizeof(uint8_t));

				BufferElement elem(semanticName, static_cast<ShaderDataType>(typeInt));
				elem.size = size;
				elem.offset = offset;
				elem.normalized = normalized != 0;
				elem.instanced = instanced != 0;
				elements.push_back(elem);
			}

			BufferLayout layout(elements);
			layout.calculateOffsetAndStride();
			spec->vertexLayout = layout;

			UUID shaderUUID = header.shaderUUID;

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<Pipeline>(handle, [spec, shaderUUID]() -> Ref<Pipeline> {
				AssetHandle<Shader> shaderHandle = AssetManager::load<Shader>(shaderUUID);
				spec->shader = AssetManager::get<Shader>(shaderHandle);

				AX_CORE_ASSERT(spec->shader, "Shader must be valid before creating pipeline!");

				return Pipeline::create(*spec);
			});

		});
	}

	void BinaryAssetLoader::loadMaterial(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<Material>().assets[handle] = nullptr;
		AssetManager::storage<Material>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- Disk IO And Parsing On Background Thread --
			if (absolutePath.empty() || !std::filesystem::exists(absolutePath)) {
				AX_CORE_LOG_ERROR("Material binary missing: {}", absolutePath.string());
				return;
			}

			std::ifstream in(absolutePath, std::ios::in | std::ios::binary);

			auto header = std::make_shared<MaterialBinaryHeader>();
			in.read(reinterpret_cast<char*>(header.get()), sizeof(MaterialBinaryHeader));

			auto textureLinks = std::make_shared<std::vector<std::pair<TextureSlot, UUID>>>();

			for (uint32_t i = 0; i < header->textureCount; i++) {
				uint32_t slotInt;
				in.read(reinterpret_cast<char*>(&slotInt), sizeof(uint32_t));

				UUID textureUUID;
				in.read(reinterpret_cast<char*>(&textureUUID), sizeof(UUID));

				textureLinks->push_back({ static_cast<TextureSlot>(slotInt), textureUUID });
			}

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<Material>(handle, [header, textureLinks]() -> Ref<Material> {
				Ref<Material> material;

				// -- Chain Pipline Load --
				if (header->pipelineUUID.isValid()) {
					AssetHandle<Pipeline> pipelineHandle = AssetManager::load<Pipeline>(header->pipelineUUID);
					material = Material::create("", pipelineHandle, header->properties);
				}
				else {
					material = Material::create("", header->properties);
				}

				// -- Chain Texture Loads --
				for (const auto& link : *textureLinks) {
					AssetHandle<Texture2D> textureHandle = AssetManager::load<Texture2D>(link.second);
					material->setTexture(link.first, textureHandle);
				}

				return material;
			});

		});
	}

	void BinaryAssetLoader::loadAudioClip(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<AudioClip>().assets[handle] = nullptr;
		AssetManager::storage<AudioClip>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- Disk IO On Background Thread --
			if (absolutePath.empty() || !std::filesystem::exists(absolutePath)) {
				AX_CORE_LOG_ERROR("Audio binary missing: {}", absolutePath.string());
				return;
			}

			std::ifstream in(absolutePath, std::ios::in | std::ios::binary);

			BinaryAssetHeader header;
			in.read(reinterpret_cast<char*>(&header), sizeof(BinaryAssetHeader));

			uint32_t modeInt;
			in.read(reinterpret_cast<char*>(&modeInt), sizeof(uint32_t));
			AudioClip::Mode mode = static_cast<AudioClip::Mode>(modeInt);

			uint64_t dataSize;
			in.read(reinterpret_cast<char*>(&dataSize), sizeof(uint64_t));

			if (mode == AudioClip::Mode::Stream) {
				AssetManager::submitToMainThread<AudioClip>(handle, [absolutePath, mode]() -> Ref<AudioClip> {
					return std::make_shared<AudioClip>(absolutePath, mode);
				});
			}
			else {
				auto audioData = std::make_shared<std::vector<uint8_t>>(dataSize);
				in.read(reinterpret_cast<char*>(audioData->data()), dataSize);

				// -- Submit To Main Thread --
				AssetManager::submitToMainThread<AudioClip>(handle, [audioData, mode]() -> Ref<AudioClip> {
					return std::make_shared<AudioClip>(std::move(*audioData), mode);
				});
			}
		});
	}

	void BinaryAssetLoader::loadPhysicsMaterial(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<PhysicsMaterial>().assets[handle] = nullptr;
		AssetManager::storage<PhysicsMaterial>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- Disk IO On Background Thread --
			if (absolutePath.empty() || !std::filesystem::exists(absolutePath)) {
				AX_CORE_LOG_ERROR("PhysicsMaterial binary missing: {}", absolutePath.string());
				return;
			}

			std::ifstream in(absolutePath, std::ios::in | std::ios::binary);

			BinaryAssetHeader header;
			in.read(reinterpret_cast<char*>(&header), sizeof(BinaryAssetHeader));

			float staticFriction = 0.0f;
			float dynamicFriction = 0.0f;
			float restitution = 0.0f;

			in.read(reinterpret_cast<char*>(&staticFriction), sizeof(float));
			in.read(reinterpret_cast<char*>(&dynamicFriction), sizeof(float));
			in.read(reinterpret_cast<char*>(&restitution), sizeof(float));

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<PhysicsMaterial>(handle, [staticFriction, dynamicFriction, restitution]() -> Ref<PhysicsMaterial> {
				Ref<PhysicsMaterial> material = std::make_shared<PhysicsMaterial>();

				material->staticFriction = staticFriction;
				material->dynamicFriction = dynamicFriction;
				material->restitution = restitution;

				return material;
			});

		});
	}

	void BinaryAssetLoader::loadPrefab(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<Prefab>().assets[handle] = nullptr;
		AssetManager::storage<Prefab>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- Disk IO On Background Thread --
			if (absolutePath.empty() || !std::filesystem::exists(absolutePath)) {
				AX_CORE_LOG_ERROR("Prefab binary missing: {}", absolutePath.string());
				return;
			}

			std::ifstream in(absolutePath, std::ios::in | std::ios::binary);
			BinaryAssetHeader header;
			in.read(reinterpret_cast<char*>(&header), sizeof(BinaryAssetHeader));

			size_t currentPos = in.tellg();
			in.seekg(0, std::ios::end);
			size_t size = static_cast<size_t>(in.tellg()) - currentPos;
			in.seekg(currentPos, std::ios::beg);

			auto binaryData = std::make_shared<std::vector<uint8_t>>(size);
			in.read(reinterpret_cast<char*>(binaryData->data()), size);

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<Prefab>(handle, [binaryData]() -> Ref<Prefab> {
				return std::make_shared<Prefab>(std::move(*binaryData));
			});
		});
	}

	void BinaryAssetLoader::loadAnimationClip(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<AnimationClip>().assets[handle] = nullptr;
		AssetManager::storage<AnimationClip>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- Disk IO And Parsing On Background Thread --
			if (absolutePath.empty() || !std::filesystem::exists(absolutePath)) {
				AX_CORE_LOG_ERROR("Animation binary missing: {}", absolutePath.string());
				return;
			}

			std::ifstream in(absolutePath, std::ios::in | std::ios::binary);
			if (!in.is_open()) {
				AX_CORE_LOG_ERROR("Failed to open Animation binary: {}", absolutePath.string());
				return;
			}

			BinaryAssetHeader header;
			in.read(reinterpret_cast<char*>(&header), sizeof(BinaryAssetHeader));

			Ref<AnimationClip> clip = std::make_shared<AnimationClip>();

			uint32_t boneAnimationCount = 0;
			in.read(reinterpret_cast<char*>(&clip->duration), sizeof(float));
			in.read(reinterpret_cast<char*>(&clip->ticksPerSecond), sizeof(float));
			in.read(reinterpret_cast<char*>(&boneAnimationCount), sizeof(uint32_t));

			clip->boneAnimations.resize(boneAnimationCount);

			for (uint32_t i = 0; i < boneAnimationCount; i++) {
				auto& boneAnimation = clip->boneAnimations[i];

				uint32_t nameLength = 0;
				in.read(reinterpret_cast<char*>(&nameLength), sizeof(uint32_t));
				if (nameLength > 0) {
					boneAnimation.boneName.resize(nameLength);
					in.read(&boneAnimation.boneName[0], nameLength);
				}

				uint32_t posCount = 0;
				in.read(reinterpret_cast<char*>(&posCount), sizeof(uint32_t));
				boneAnimation.positions.resize(posCount);
				for (uint32_t k = 0; k < posCount; ++k) {
					in.read(reinterpret_cast<char*>(&boneAnimation.positions[k].time), sizeof(float));
					in.read(reinterpret_cast<char*>(&boneAnimation.positions[k].value), sizeof(DirectX::XMFLOAT3));
				}

				uint32_t rotCount = 0;
				in.read(reinterpret_cast<char*>(&rotCount), sizeof(uint32_t));
				boneAnimation.rotations.resize(rotCount);
				for (uint32_t k = 0; k < rotCount; ++k) {
					in.read(reinterpret_cast<char*>(&boneAnimation.rotations[k].time), sizeof(float));
					in.read(reinterpret_cast<char*>(&boneAnimation.rotations[k].value), sizeof(DirectX::XMFLOAT4));
				}

				uint32_t scaCount = 0;
				in.read(reinterpret_cast<char*>(&scaCount), sizeof(uint32_t));
				boneAnimation.scales.resize(scaCount);
				for (uint32_t k = 0; k < scaCount; ++k) {
					in.read(reinterpret_cast<char*>(&boneAnimation.scales[k].time), sizeof(float));
					in.read(reinterpret_cast<char*>(&boneAnimation.scales[k].value), sizeof(DirectX::XMFLOAT3));
				}
			}

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<AnimationClip>(handle, [clip]() -> Ref<AnimationClip> {
				return clip;
			});

		});
	}

	void BinaryAssetLoader::loadSkeletalMesh(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<SkeletalMesh>().assets[handle] = nullptr;
		AssetManager::storage<SkeletalMesh>().handleToPath[handle] = absolutePath;

		JobSystem::submit([handle, absolutePath]() {

			// -- Disk IO And Parsing On Background Thread --
			if (absolutePath.empty() || !std::filesystem::exists(absolutePath)) {
				AX_CORE_LOG_ERROR("Binary file path is missing or invalid: {}", absolutePath.string());
				return;
			}

			std::ifstream in(absolutePath, std::ios::in | std::ios::binary);
			if (!in) {
				AX_CORE_LOG_ERROR("Failed to read SkeletalMesh data. File is corrupted or empty: {}", absolutePath.string());
				return;
			}

			BinaryAssetHeader header;
			in.read(reinterpret_cast<char*>(&header), sizeof(BinaryAssetHeader));

			uint32_t vertexCount = 0, indexCount = 0, submeshCount = 0, boneCount = 0;
			in.read(reinterpret_cast<char*>(&vertexCount), sizeof(uint32_t));
			in.read(reinterpret_cast<char*>(&indexCount), sizeof(uint32_t));
			in.read(reinterpret_cast<char*>(&submeshCount), sizeof(uint32_t));
			in.read(reinterpret_cast<char*>(&boneCount), sizeof(uint32_t));

			auto meshData = std::make_shared<SkeletalMeshData>();
			meshData->vertices.resize(vertexCount);
			meshData->indices.resize(indexCount);

			in.read(reinterpret_cast<char*>(meshData->vertices.data()), vertexCount * sizeof(SkeletalVertex));
			in.read(reinterpret_cast<char*>(meshData->indices.data()), indexCount * sizeof(uint32_t));

			if (submeshCount > 0) {
				meshData->submeshes.resize(submeshCount);
				in.read(reinterpret_cast<char*>(meshData->submeshes.data()), submeshCount * sizeof(Submesh));
			}

			meshData->skeleton.bones.resize(boneCount);
			for (uint32_t i = 0; i < boneCount; i++) {
				auto& bone = meshData->skeleton.bones[i];

				uint32_t nameLength;
				in.read(reinterpret_cast<char*>(&nameLength), sizeof(uint32_t));
				bone.name.resize(nameLength);
				in.read(&bone.name[0], nameLength);

				in.read(reinterpret_cast<char*>(&bone.parentIndex), sizeof(int32_t));
				in.read(reinterpret_cast<char*>(&bone.localBindTransform), sizeof(DirectX::XMMATRIX));
				in.read(reinterpret_cast<char*>(&bone.inverseBindMatrix), sizeof(DirectX::XMMATRIX));

				uint32_t childCount;
				in.read(reinterpret_cast<char*>(&childCount), sizeof(uint32_t));

				if (childCount > 0) {
					bone.children.resize(childCount);
					in.read(reinterpret_cast<char*>(bone.children.data()), childCount * sizeof(int));
				}
			}

			in.read(reinterpret_cast<char*>(&meshData->skeleton.rootTransform), sizeof(DirectX::XMMATRIX));

			// -- Submit To Main Thread --
			AssetManager::submitToMainThread<SkeletalMesh>(handle, [meshData]() -> Ref<SkeletalMesh> {
				return SkeletalMesh::create(*meshData);
			});

		});
	}

	void BinaryAssetLoader::reloadMaterial(UUID handle, const std::filesystem::path& absolutePath) {
		AX_CORE_LOG_WARN("Reloading Assets is disabled in Binary mode!");
	}

}
