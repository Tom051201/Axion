#include "axpch.h"
#include "BinaryAssetLoader.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/BinaryHeaders.h"
#include "AxionEngine/Source/scene/Skybox.h"
#include "AxionEngine/Source/scene/Prefab.h"
#include "AxionEngine/Source/scene/Animation.h"
#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/SkeletalMesh.h"
#include "AxionEngine/Source/render/Shader.h"
#include "AxionEngine/Source/render/Material.h"
#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/render/Texture.h"
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

		AssetManager::storage<Mesh>().loadQueue.push_back({ handle,
			[absolutePath]() -> Ref<Mesh> {

				if (absolutePath.empty() || !std::filesystem::exists(absolutePath)) {
					AX_CORE_LOG_ERROR("Mesh binary missing: {}", absolutePath.string());
					return nullptr;
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
					return nullptr;
				}

				MeshData meshData;
				meshData.vertices.resize(vertexCount);
				meshData.indices.resize(indexCount);

				in.read(reinterpret_cast<char*>(meshData.vertices.data()), vertexCount * sizeof(Vertex));
				in.read(reinterpret_cast<char*>(meshData.indices.data()), indexCount * sizeof(uint32_t));

				if (submeshCount > 0) {
					meshData.submeshes.resize(submeshCount);
					in.read(reinterpret_cast<char*>(meshData.submeshes.data()), submeshCount * sizeof(Submesh));
				}

				return Mesh::create(meshData);
			}
		});

		AssetManager::storage<Mesh>().handleToPath[handle] = absolutePath;
	}

	void BinaryAssetLoader::loadTexture2D(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<Texture2D>().assets[handle] = nullptr;
		AssetManager::storage<Texture2D>().handleToPath[handle] = absolutePath;
		AssetManager::storage<Texture2D>().loadQueue.push_back({ handle,
			[absolutePath]() -> Ref<Texture2D> {
		
				if (absolutePath.empty() || !std::filesystem::exists(absolutePath)) {
					AX_CORE_LOG_ERROR("Texture binary missing: {}", absolutePath.string());
					return nullptr;
				}
		
				std::ifstream in(absolutePath, std::ios::in | std::ios::binary);
		
				BinaryAssetHeader header;
				in.read(reinterpret_cast<char*>(&header), sizeof(BinaryAssetHeader));
		
				uint64_t dataSize = 0;
				in.read(reinterpret_cast<char*>(&dataSize), sizeof(uint64_t));
		
				if (dataSize == 0 || dataSize > 100000000) {
					AX_CORE_LOG_ERROR("Texture Corrupted or Missing! Size: {} bytes. Path: {}", dataSize, absolutePath.string());
					return nullptr;
				}
		
				std::vector<uint8_t> imageData(dataSize);
				in.read(reinterpret_cast<char*>(imageData.data()), dataSize);
		
				return Texture2D::create(imageData.data(), imageData.size());
			}
		});
	}

	void BinaryAssetLoader::loadTextureCube(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<TextureCube>().assets[handle] = nullptr;

		AssetManager::storage<TextureCube>().loadQueue.push_back({ handle,
			[absolutePath]() {
				std::ifstream in(absolutePath, std::ios::in | std::ios::binary);

				BinaryAssetHeader header;
				in.read(reinterpret_cast<char*>(&header), sizeof(BinaryAssetHeader));

				uint64_t dataSize;
				in.read(reinterpret_cast<char*>(&dataSize), sizeof(uint64_t));

				std::vector<uint8_t> imageData(dataSize);
				in.read(reinterpret_cast<char*>(imageData.data()), dataSize);

				return TextureCube::create(imageData.data(), imageData.size());
			}
		});

		AssetManager::storage<TextureCube>().handleToPath[handle] = absolutePath;
	}

	void BinaryAssetLoader::loadSkybox(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<Skybox>().assets[handle] = nullptr;

		AssetManager::storage<Skybox>().loadQueue.push_back({ handle,
			[absolutePath]() {
				std::ifstream in(absolutePath, std::ios::in | std::ios::binary);

				SkyboxBinaryHeader header;
				in.read(reinterpret_cast<char*>(&header), sizeof(SkyboxBinaryHeader));

				AssetHandle<TextureCube> texHandle = AssetManager::load<TextureCube>(header.textureCubeUUID);
				if (header.pipelineUUID.isValid()) {
					AssetHandle<Pipeline> pipeHandle = AssetManager::load<Pipeline>(header.pipelineUUID);
					return std::make_shared<Skybox>(texHandle, pipeHandle);
				}
				else {
					return std::make_shared<Skybox>(texHandle);
				}
			}
		});

		AssetManager::storage<Skybox>().handleToPath[handle] = absolutePath;
	}

	void BinaryAssetLoader::loadShader(UUID handle, const std::filesystem::path& absolutePath) {
		std::ifstream in(absolutePath, std::ios::in | std::ios::binary);

		ShaderBinaryHeader header;
		in.read(reinterpret_cast<char*>(&header), sizeof(ShaderBinaryHeader));

		ShaderSpecification spec = {};
		spec.name = "RuntimeShader_" + header.assetHeader.uuid.toString();
		spec.batchTextures = header.batchTextures;

		Ref<Shader> shader = Shader::create(spec);

		std::vector<uint8_t> vsData(header.vsSize);
		in.read(reinterpret_cast<char*>(vsData.data()), header.vsSize);
		std::vector<uint8_t> psData(header.psSize);
		in.read(reinterpret_cast<char*>(psData.data()), header.psSize);

		shader->loadFromBytecode(vsData.data(), header.vsSize, psData.data(), header.psSize);

		AssetManager::storage<Shader>().assets[handle] = shader;
		AssetManager::storage<Shader>().handleToPath[handle] = absolutePath;
	}

	void BinaryAssetLoader::loadPipeline(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<Pipeline>().assets[handle] = nullptr;
		AssetManager::storage<Pipeline>().loadQueue.push_back({ handle,
			[absolutePath]() {
				std::ifstream in(absolutePath, std::ios::in | std::ios::binary);

				PipelineBinaryHeader header;
				in.read(reinterpret_cast<char*>(&header), sizeof(PipelineBinaryHeader));

				PipelineSpecification spec = {};
				spec.colorFormat = static_cast<ColorFormat>(header.colorFormat);
				spec.depthStencilFormat = static_cast<DepthStencilFormat>(header.depthStencilFormat);
				spec.depthTest = header.depthTest != 0;
				spec.depthWrite = header.depthWrite != 0;
				spec.depthFunction = static_cast<DepthCompare>(header.depthFunction);
				spec.stencilEnabled = header.stencilEnabled != 0;
				spec.sampleCount = header.sampleCount;
				spec.cullMode = static_cast<CullMode>(header.cullMode);
				spec.topology = static_cast<PrimitiveTopology>(header.topology);
				spec.numRenderTargets = header.numRenderTargets;

				std::vector<BufferElement> elements;
				elements.reserve(header.bufferElementCount);

				for (uint32_t i = 0; i < header.bufferElementCount; i++) {
					uint32_t nameLen;
					in.read(reinterpret_cast<char*>(&nameLen), sizeof(uint32_t));
					std::string semanticName(nameLen, '\0');
					in.read(&semanticName[0], nameLen);

					uint32_t typeInt;
					uint32_t size, offset;
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
				spec.vertexLayout = layout;

				AssetHandle<Shader> shaderHandle = AssetManager::load<Shader>(header.shaderUUID);
				spec.shader = AssetManager::get<Shader>(shaderHandle);

				AX_CORE_ASSERT(spec.shader, "Shader must be valid before creating pipeline!");
				return Pipeline::create(spec);
			}
		});
		AssetManager::storage<Pipeline>().handleToPath[handle] = absolutePath;
	}

	void BinaryAssetLoader::loadMaterial(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<Material>().assets[handle] = nullptr;
		AssetManager::storage<Material>().loadQueue.push_back({ handle,
			[absolutePath]() {
				std::ifstream in(absolutePath, std::ios::in | std::ios::binary);

				MaterialBinaryHeader header;
				in.read(reinterpret_cast<char*>(&header), sizeof(MaterialBinaryHeader));

				Ref<Material> material;
				if (header.pipelineUUID.isValid()) {
					AssetHandle<Pipeline> pipelineHandle = AssetManager::load<Pipeline>(header.pipelineUUID);
					material = Material::create("", pipelineHandle, header.properties);
				}
				else {
					material = Material::create("", header.properties);
				}

				for (uint32_t i = 0; i < header.textureCount; i++) {
					uint32_t slotInt;
					in.read(reinterpret_cast<char*>(&slotInt), sizeof(uint32_t));
					TextureSlot slot = static_cast<TextureSlot>(slotInt);

					UUID textureUUID;
					in.read(reinterpret_cast<char*>(&textureUUID), sizeof(UUID));

					AssetHandle<Texture2D> textureHandle = AssetManager::load<Texture2D>(textureUUID);
					material->setTexture(slot, textureHandle);
				}

				return material;
			}
		});
		AssetManager::storage<Material>().handleToPath[handle] = absolutePath;
	}

	void BinaryAssetLoader::loadAudioClip(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<AudioClip>().assets[handle] = nullptr;
		AssetManager::storage<AudioClip>().loadQueue.push_back({ handle,
			[absolutePath]() {
				std::ifstream in(absolutePath, std::ios::in | std::ios::binary);

				BinaryAssetHeader header;
				in.read(reinterpret_cast<char*>(&header), sizeof(BinaryAssetHeader));

				uint32_t modeInt;
				in.read(reinterpret_cast<char*>(&modeInt), sizeof(uint32_t));
				AudioClip::Mode mode = static_cast<AudioClip::Mode>(modeInt);

				uint64_t dataSize;
				in.read(reinterpret_cast<char*>(&dataSize), sizeof(uint64_t));

				std::vector<uint8_t> audioData(dataSize);
				in.read(reinterpret_cast<char*>(audioData.data()), dataSize);

				return std::make_shared<AudioClip>(std::move(audioData), mode);
			}
		});
		AssetManager::storage<AudioClip>().handleToPath[handle] = absolutePath;
	}

	void BinaryAssetLoader::loadPhysicsMaterial(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<PhysicsMaterial>().assets[handle] = nullptr;
		AssetManager::storage<PhysicsMaterial>().loadQueue.push_back({ handle,
			[absolutePath]() {
				std::ifstream in(absolutePath, std::ios::in | std::ios::binary);

				BinaryAssetHeader header;
				in.read(reinterpret_cast<char*>(&header), sizeof(BinaryAssetHeader));

				Ref<PhysicsMaterial> material = std::make_shared<PhysicsMaterial>();

				in.read(reinterpret_cast<char*>(&material->staticFriction), sizeof(float));
				in.read(reinterpret_cast<char*>(&material->dynamicFriction), sizeof(float));
				in.read(reinterpret_cast<char*>(&material->restitution), sizeof(float));

				return material;
			}
		});
		AssetManager::storage<PhysicsMaterial>().handleToPath[handle] = absolutePath;
	}

	void BinaryAssetLoader::loadPrefab(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<Prefab>().assets[handle] = nullptr;
		AssetManager::storage<Prefab>().loadQueue.push_back({ handle,
			[absolutePath]() {
				std::ifstream in(absolutePath, std::ios::in | std::ios::binary);
				BinaryAssetHeader header;
				in.read(reinterpret_cast<char*>(&header), sizeof(BinaryAssetHeader));

				size_t currentPos = in.tellg();
				in.seekg(0, std::ios::end);
				size_t size = static_cast<size_t>(in.tellg()) - currentPos;
				in.seekg(currentPos, std::ios::beg);

				std::vector<uint8_t> binaryData(size);
				in.read(reinterpret_cast<char*>(binaryData.data()), size);

				return std::make_shared<Prefab>(std::move(binaryData));
			}
		});
		AssetManager::storage<Prefab>().handleToPath[handle] = absolutePath;
	}

	void BinaryAssetLoader::loadAnimationClip(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<AnimationClip>().assets[handle] = nullptr;
		AssetManager::storage<AnimationClip>().loadQueue.push_back({ handle,
			[absolutePath]() -> Ref<AnimationClip> {

				if (absolutePath.empty() || !std::filesystem::exists(absolutePath)) {
					AX_CORE_LOG_ERROR("Animation binary missing: {}", absolutePath.string());
					return nullptr;
				}

				std::ifstream in(absolutePath, std::ios::in  | std::ios::binary);
				if (!in.is_open()) {
					AX_CORE_LOG_ERROR("Failed to open Animation binary: {}", absolutePath.string());
					return nullptr;
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

				return clip;
			}
		});
		AssetManager::storage<AnimationClip>().handleToPath[handle] = absolutePath;
	}

	void BinaryAssetLoader::loadSkeletalMesh(UUID handle, const std::filesystem::path& absolutePath) {
		AssetManager::storage<SkeletalMesh>().assets[handle] = nullptr;
		AssetManager::storage<SkeletalMesh>().loadQueue.push_back({ handle,
			[absolutePath]() -> Ref<SkeletalMesh> {

				if (absolutePath.empty() || !std::filesystem::exists(absolutePath)) {
					AX_CORE_LOG_ERROR("Binary file path is missing of invalid: {}", absolutePath.string());
					return nullptr;
				}

				std::ifstream in(absolutePath, std::ios::in | std::ios::binary);
				BinaryAssetHeader header;
				in.read(reinterpret_cast<char*>(&header), sizeof(BinaryAssetHeader));

				uint32_t vertexCount = 0, indexCount = 0, submeshCount = 0, boneCount = 0;
				in.read(reinterpret_cast<char*>(&vertexCount), sizeof(uint32_t));
				in.read(reinterpret_cast<char*>(&indexCount), sizeof(uint32_t));
				in.read(reinterpret_cast<char*>(&submeshCount), sizeof(uint32_t));
				in.read(reinterpret_cast<char*>(&boneCount), sizeof(uint32_t));

				if (!in) {
					AX_CORE_LOG_ERROR("Failed to read SkeletalMesh data. File is corrupted or empty: {}", absolutePath.string());
					return nullptr;
				}

				SkeletalMeshData meshData;
				meshData.vertices.resize(vertexCount);
				meshData.indices.resize(indexCount);

				in.read(reinterpret_cast<char*>(meshData.vertices.data()), vertexCount * sizeof(SkeletalVertex));
				in.read(reinterpret_cast<char*>(meshData.indices.data()), indexCount * sizeof(uint32_t));

				if (submeshCount > 0) {
					meshData.submeshes.resize(submeshCount);
					in.read(reinterpret_cast<char*>(meshData.submeshes.data()), submeshCount * sizeof(Submesh));
				}

				meshData.skeleton.bones.resize(boneCount);
				for (uint32_t i = 0; i < boneCount; i++) {
					auto& bone = meshData.skeleton.bones[i];

					uint32_t nameLength;
					in.read(reinterpret_cast<char*>(&nameLength), sizeof(uint32_t));
					bone.name.resize(nameLength);
					in.read(&bone.name[0], nameLength);

					in.read(reinterpret_cast<char*>(&bone.parentIndex), sizeof(int32_t));
					in.read(reinterpret_cast<char*>(&bone.localBindTransform), sizeof(DirectX::XMMATRIX));
					in.read(reinterpret_cast<char*>(&bone.inverseBindMatrix), sizeof(DirectX::XMMATRIX));
					
					uint32_t childCount;
					in.read(reinterpret_cast<char*>(&childCount), sizeof(uint32_t));
					AX_CORE_LOG_INFO("Reading bone '{}'. Child count: {}", bone.name, childCount);
					if (childCount > 0) {
						bone.children.resize(childCount);
						in.read(reinterpret_cast<char*>(bone.children.data()), childCount * sizeof(int));
					}
				}

				in.read(reinterpret_cast<char*>(&meshData.skeleton.rootTransform), sizeof(DirectX::XMMATRIX));

				return SkeletalMesh::create(meshData);
			}
		});
		AssetManager::storage<SkeletalMesh>().handleToPath[handle] = absolutePath;
	}

	void BinaryAssetLoader::reloadMaterial(UUID handle, const std::filesystem::path& absolutePath) {
		AX_CORE_LOG_WARN("Reloading Assets is disabled in Binary mode!");
	}

}
