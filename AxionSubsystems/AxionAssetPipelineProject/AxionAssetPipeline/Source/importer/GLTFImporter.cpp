#include "GLTFImporter.h"

#define CGLTF_IMPLEMENTATION
#include "AxionAssetPipeline/Vendor/cgltf/cgltf.h"

#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/scene/Entity.h"
#include "AxionEngine/Source/scene/Components.h"

#include "AxionAssetPipeline/Source/AxTexture2D.h"
#include "AxionAssetPipeline/Source/AxMaterial.h"
#include "AxionAssetPipeline/Source/AxMesh.h"
#include "AxionAssetPipeline/Source/AxPrefab.h"
#include "AxionAssetPipeline/Source/AxSkeletalMesh.h"
#include "AxionAssetPipeline/Source/AxAnimationClip.h"

#include <fstream>
#include <unordered_map>

namespace Axion::AAP {

	void GLTFImporter::import(const std::filesystem::path& glbPath) {

		cgltf_options options = {};
		cgltf_data* data = nullptr;
		cgltf_result result = cgltf_parse_file(&options, glbPath.string().c_str(), &data);

		if (result != cgltf_result_success) {
			AX_CORE_LOG_ERROR("GLTFImporter: Failed to parse {}", glbPath.string());
			return;
		}

		result = cgltf_load_buffers(&options, data, glbPath.string().c_str());
		if (result != cgltf_result_success) {
			AX_CORE_LOG_ERROR("GLTFImporter: Failed to load buffers for {}", glbPath.string());
			cgltf_free(data);
			return;
		}

		std::string baseName = glbPath.stem().string();
		auto assetRegistry = ProjectManager::getProject()->getAssetRegistry();

		// ----- Phase 1: Extract Textures -----
		std::filesystem::path texOutputDir = ProjectManager::getProject()->getAssetsPath() / "textures" / baseName;
		std::filesystem::create_directories(texOutputDir);
		AX_CORE_LOG_INFO("Unpacking GLB Textures: {} (Found {} images)", baseName, data->images_count);
		std::vector<UUID> extractedTextureUUIDs(data->images_count, UUID(0, 0));

		for (cgltf_size i = 0; i < data->images_count; ++i) {
			cgltf_image& image = data->images[i];

			std::string extension = ".png";
			std::string texName = image.name ? image.name : (baseName + "_Tex_" + std::to_string(i));
			std::filesystem::path outTexPath;
			bool textureSuccessfullySaved = false;

			// -- Embedded Image --
			if (image.buffer_view) {
				uint8_t* imgData = (uint8_t*)image.buffer_view->buffer->data + image.buffer_view->offset;
				cgltf_size imgSize = image.buffer_view->size;

				if (image.mime_type && std::string(image.mime_type) == "image/jpeg") {
					extension = ".jpg";
				}

				outTexPath = texOutputDir / (texName + extension);
				std::ofstream outStream(outTexPath, std::ios::out | std::ios::binary);
				if (outStream) {
					outStream.write(reinterpret_cast<const char*>(imgData), imgSize);
					outStream.close();
					textureSuccessfullySaved = true;
				}
			}
			// -- External Image File --
			else if (image.uri) {
				std::filesystem::path sourceTexPath = glbPath.parent_path() / image.uri;
				extension = sourceTexPath.extension().string();

				outTexPath = texOutputDir / (texName + extension);

				if (std::filesystem::exists(sourceTexPath)) {
					try {
						std::filesystem::copy_file(sourceTexPath, outTexPath, std::filesystem::copy_options::overwrite_existing);
						textureSuccessfullySaved = true;
					}
					catch (const std::exception& e) {
						AX_CORE_LOG_ERROR("Failed to copy GLTF texture '{}': {}", sourceTexPath.string(), e.what());
					}
				}
				else {
					AX_CORE_LOG_WARN("GLTF texture file not found: {}", sourceTexPath.string());
				}
			}

			// -- Generate Engine Asset --
			if (textureSuccessfullySaved) {
				UUID texUUID = UUID::generate();
				extractedTextureUUIDs[i] = texUUID;

				// -- Create .axtex file --
				Texture2DAssetData textureAssetData;
				textureAssetData.uuid = texUUID;
				textureAssetData.name = texName;
				if (extension == ".jpg" || extension == ".jpeg") textureAssetData.fileFormat = TextureFormat::JPG;
				else textureAssetData.fileFormat = TextureFormat::PNG;
				textureAssetData.filePath = AssetManager::getRelativeToAssets(outTexPath);

				std::filesystem::path axtexPath = texOutputDir / (texName + ".axtex");
				Texture2DParser::createTextFile(textureAssetData, axtexPath);

				// -- Register in Asset Registry --
				AssetMetadata metadata;
				metadata.handle = texUUID;
				metadata.type = AssetType::Texture2D;
				metadata.filePath = AssetManager::getRelativeToAssets(axtexPath);
				assetRegistry->add(metadata);

				AX_CORE_LOG_TRACE("Extracted Texture: {}", axtexPath.filename().string());
			}
		}



		// ----- Phase 2: Extract Materials -----
		std::filesystem::path matOutputDir = ProjectManager::getProject()->getAssetsPath() / "materials" / baseName;
		std::filesystem::create_directories(matOutputDir);
		AX_CORE_LOG_INFO("Unpacking GLB Materials: {} (Found {} materials)", baseName, data->materials_count);
		std::vector<UUID> extractedMaterialUUIDs(data->materials_count, UUID(0, 0));

		for (cgltf_size i = 0; i < data->materials_count; ++i) {
			cgltf_material& mat = data->materials[i];
			std::string matName = mat.name ? mat.name : (baseName + "_Mat_" + std::to_string(i));

			MaterialAssetData matAssetData;
			matAssetData.uuid = UUID::generate();
			matAssetData.name = matName;
			extractedMaterialUUIDs[i] = matAssetData.uuid;

			// -- Read PBR Properties --
			if (mat.has_pbr_metallic_roughness) {
				auto& pbr = mat.pbr_metallic_roughness;

				matAssetData.properties.albedoColor = { pbr.base_color_factor[0], pbr.base_color_factor[1], pbr.base_color_factor[2], pbr.base_color_factor[3] };
				matAssetData.properties.metalness = pbr.metallic_factor;
				matAssetData.properties.roughness = pbr.roughness_factor;

				// -- Albedo Texture --
				if (pbr.base_color_texture.texture && pbr.base_color_texture.texture->image) {
					cgltf_size imgIndex = pbr.base_color_texture.texture->image - data->images;
					UUID texUUID = extractedTextureUUIDs[imgIndex];
					if (texUUID.isValid()) {
						matAssetData.textures[TextureSlot::Albedo] = assetRegistry->get(texUUID).filePath.string();
					}
				}

				// -- Metalness and Roughness Texture --
				if (pbr.metallic_roughness_texture.texture && pbr.metallic_roughness_texture.texture->image) {
					cgltf_size imgIndex = pbr.metallic_roughness_texture.texture->image - data->images;
					UUID texUUID = extractedTextureUUIDs[imgIndex];
					if (texUUID.isValid()) {
						matAssetData.textures[TextureSlot::Metalness] = assetRegistry->get(texUUID).filePath.string();
						matAssetData.textures[TextureSlot::Roughness] = assetRegistry->get(texUUID).filePath.string();
						matAssetData.properties.useMetalnessMap = 1.0f;
						matAssetData.properties.useRoughnessMap = 1.0f;
					}
				}

			}

			// -- Normal Map --
			if (mat.normal_texture.texture && mat.normal_texture.texture->image) {
				cgltf_size imgIndex = mat.normal_texture.texture->image - data->images;
				UUID texUUID = extractedTextureUUIDs[imgIndex];
				if (texUUID.isValid()) {
					matAssetData.textures[TextureSlot::Normal] = assetRegistry->get(texUUID).filePath.string();
					matAssetData.properties.useNormalMap = 1.0f;
				}
			}

			// -- Emissive Map and Factor --
			matAssetData.properties.emissionStrength = (mat.emissive_factor[0] + mat.emissive_factor[1] + mat.emissive_factor[2]) / 3.0f;
			if (mat.emissive_texture.texture && mat.emissive_texture.texture->image) {
				cgltf_size imgIndex = mat.emissive_texture.texture->image - data->images;
				UUID texUUID = extractedTextureUUIDs[imgIndex];
				if (texUUID.isValid()) {
					matAssetData.textures[TextureSlot::Emissive] = assetRegistry->get(texUUID).filePath.string();
					matAssetData.properties.useEmissiveMap = 1.0f;
				}
			}

			// -- Occlusion Map --
			if (mat.occlusion_texture.texture && mat.occlusion_texture.texture->image) {
				cgltf_size imgIndex = mat.occlusion_texture.texture->image - data->images;
				UUID texUUID = extractedTextureUUIDs[imgIndex];
				if (texUUID.isValid()) {
					matAssetData.textures[TextureSlot::Occlusion] = assetRegistry->get(texUUID).filePath.string();
					matAssetData.properties.useOcclusionMap = 1.0f;
				}
			}

			// -- Create .axmat file --
			std::filesystem::path axmatPath = matOutputDir / (matName + ".axmat");
			MaterialParser::createTextFile(matAssetData, axmatPath);

			// -- Register in Asset Registry --
			AssetMetadata metadata;
			metadata.handle = matAssetData.uuid;
			metadata.type = AssetType::Material;
			metadata.filePath = AssetManager::getRelativeToAssets(axmatPath);
			assetRegistry->add(metadata);

			AX_CORE_LOG_TRACE("Extracted Material: {}", axmatPath.filename().string());
		}



		// -- Phase 3: Extract Geometry --
		std::filesystem::path meshOutputDir = ProjectManager::getProject()->getAssetsPath() / "meshes" / baseName;
		std::filesystem::create_directories(meshOutputDir);
		AX_CORE_LOG_INFO("Unpacking GLTF/GLB Geometry: {}", baseName);

		std::string originalExt = glbPath.extension().string();
		std::filesystem::path copiedModelPath = meshOutputDir / (baseName + originalExt);

		if (std::filesystem::absolute(glbPath) != std::filesystem::absolute(copiedModelPath)) {
			try {
				std::filesystem::copy_file(glbPath, copiedModelPath, std::filesystem::copy_options::overwrite_existing);
				AX_CORE_LOG_TRACE("Copied source model to: {}", copiedModelPath.string());

				for (cgltf_size i = 0; i < data->buffers_count; ++i) {
					if (data->buffers[i].uri) {
						std::string uri = data->buffers[i].uri;

						if (uri.find("data:") != 0) {
							std::filesystem::path sourceBufferPath = glbPath.parent_path() / uri;
							std::filesystem::path destBufferPath = meshOutputDir / uri;

							if (std::filesystem::exists(sourceBufferPath)) {
								std::filesystem::copy_file(sourceBufferPath, destBufferPath, std::filesystem::copy_options::overwrite_existing);
								AX_CORE_LOG_TRACE("Copied external buffer to: {}", destBufferPath.filename().string());
							}
							else {
								AX_CORE_LOG_ERROR("Missing external buffer file: {}", sourceBufferPath.string());
							}
						}
					}
				}
			}
			catch (const std::exception& e) {
				AX_CORE_LOG_ERROR("Failed to copy model or buffer files: {}", e.what());
			}
		}

		// -- Check if skeletal --
		bool isSkeletal = data->skins_count > 0;
		UUID mainMeshUUID = UUID::generate();

		if (isSkeletal) {
			SkeletalMeshAssetData skelAssetData;
			skelAssetData.uuid = mainMeshUUID;
			skelAssetData.name = baseName;
			//skelAssetData.fileFormat = (originalExt == ".glb") ? MeshFormat::GLB : MeshFormat::GLTF;
			skelAssetData.filePath = AssetManager::getRelativeToAssets(copiedModelPath);

			std::filesystem::path axmeshPath = meshOutputDir / (baseName + ".axskelmesh");
			SkeletalMeshParser::createTextFile(skelAssetData, axmeshPath);

			AssetMetadata meshMetadata;
			meshMetadata.handle = skelAssetData.uuid;
			meshMetadata.type = AssetType::SkeletalMesh;
			meshMetadata.filePath = AssetManager::getRelativeToAssets(axmeshPath);
			assetRegistry->add(meshMetadata);
			AX_CORE_LOG_TRACE("Extracted Skeletal Mesh: {}", axmeshPath.filename().string());
		}
		else {
			MeshAssetData meshAssetData;
			meshAssetData.uuid = mainMeshUUID;
			meshAssetData.name = baseName;
			meshAssetData.fileFormat = (originalExt == ".glb") ? MeshFormat::GLB : MeshFormat::GLTF;
			meshAssetData.filePath = AssetManager::getRelativeToAssets(copiedModelPath);

			std::filesystem::path axmeshPath = meshOutputDir / (baseName + ".axmesh");
			MeshParser::createTextFile(meshAssetData, axmeshPath);

			AssetMetadata meshMetadata;
			meshMetadata.handle = meshAssetData.uuid;
			meshMetadata.type = AssetType::Mesh;
			meshMetadata.filePath = AssetManager::getRelativeToAssets(axmeshPath);
			assetRegistry->add(meshMetadata);
			AX_CORE_LOG_TRACE("Extracted Static Mesh: {}", axmeshPath.filename().string());
		}

		// -- Phase 4: Extract Animations --
		std::vector<UUID> extractedAnimationUUIDs;
		if (data->animations_count > 0) {
			std::filesystem::path animOutputDir = ProjectManager::getProject()->getAssetsPath() / "animations" / baseName;
			std::filesystem::create_directories(animOutputDir);
			AX_CORE_LOG_INFO("Unpacking GLB Animations: {} (Found {})", baseName, data->animations_count);

			for (cgltf_size i = 0; i < data->animations_count; ++i) {
				std::string animName = data->animations[i].name ? data->animations[i].name : (baseName + "_Anim_" + std::to_string(i));

				AnimationClipAssetData animAssetData;
				animAssetData.uuid = UUID::generate();
				animAssetData.name = animName;
				animAssetData.filePath = AssetManager::getRelativeToAssets(copiedModelPath);

				std::filesystem::path axanimPath = animOutputDir / (animName + ".axanim");
				AnimationClipParser::createTextFile(animAssetData, axanimPath);

				AssetMetadata animMetadata;
				animMetadata.handle = animAssetData.uuid;
				animMetadata.type = AssetType::AnimationClip;
				animMetadata.filePath = AssetManager::getRelativeToAssets(axanimPath);
				assetRegistry->add(animMetadata);

				extractedAnimationUUIDs.push_back(animAssetData.uuid);
				AX_CORE_LOG_TRACE("Extracted Animation: {}", axanimPath.filename().string());
			}
		}


		// -- Create Prefab --
		std::filesystem::path prefabOutputDir = ProjectManager::getProject()->getAssetsPath() / "prefabs" / baseName;
		std::filesystem::create_directories(prefabOutputDir);
		AX_CORE_LOG_INFO("Generating GLB Prefab: {}", baseName);

		Ref<Scene> tempScene = std::make_shared<Scene>();
		Entity prefabEntity = tempScene->createEntity(baseName);

		if (isSkeletal) {
			AssetHandle<SkeletalMesh> skelHandle;
			skelHandle.uuid = mainMeshUUID;
			prefabEntity.addComponent<SkeletalMeshComponent>(skelHandle);

			if (!extractedAnimationUUIDs.empty()) {
				AssetHandle<AnimationClip> animHandle;
				animHandle.uuid = extractedAnimationUUIDs[0];

				AnimatorComponent animComp;
				animComp.currentClip = animHandle;
				animComp.isPlaying = true;
				prefabEntity.addComponent<AnimatorComponent>(animComp);
			}
		}
		else {
			AssetHandle<Mesh> meshHandle;
			meshHandle.uuid = mainMeshUUID;
			prefabEntity.addComponent<MeshComponent>(meshHandle);
		}

		auto& matComp = prefabEntity.addComponent<MaterialComponent>();
		matComp.materials.clear();

		for (cgltf_size m = 0; m < data->meshes_count; ++m) {
			const cgltf_mesh& mesh = data->meshes[m];
			for (cgltf_size p = 0; p < mesh.primitives_count; ++p) {
				const cgltf_primitive& primitive = mesh.primitives[p];

				AssetHandle<Material> matHandle;
				if (primitive.material) {
					cgltf_size matIndex = primitive.material - data->materials;
					if (matIndex < extractedMaterialUUIDs.size()) {
						matHandle.uuid = extractedMaterialUUIDs[matIndex];
					}
				}
				matComp.materials.push_back(matHandle);
			}
		}

		UUID prefabUUID = UUID::generate();
		PrefabAssetData prefabAssetData;
		prefabAssetData.uuid = prefabUUID;
		prefabAssetData.name = baseName;
		prefabAssetData.scene = tempScene;
		prefabAssetData.entity = prefabEntity;

		std::filesystem::path axprefabPath = prefabOutputDir / (baseName + ".axprefab");
		PrefabParser::createTextFile(prefabAssetData, axprefabPath);

		AssetMetadata prefabMetadata;
		prefabMetadata.handle = prefabUUID;
		prefabMetadata.type = AssetType::Prefab;
		prefabMetadata.filePath = AssetManager::getRelativeToAssets(axprefabPath);
		assetRegistry->add(prefabMetadata);

		AX_CORE_LOG_TRACE("Created Prefab: {}", axprefabPath.filename().string());



		// -- Save the Asset Registry --
		assetRegistry->serialize(ProjectManager::getProject()->getProjectPath() / "AssetRegistry.yaml");

		cgltf_free(data);
	}

	MeshData GLTFImporter::extractMeshes(const std::filesystem::path& path) {
		MeshData meshData;

		cgltf_options options = {};
		cgltf_data* data = nullptr;
		cgltf_result result = cgltf_parse_file(&options, path.string().c_str(), &data);

		if (result != cgltf_result_success) {
			AX_CORE_LOG_ERROR("Failed to parse GLTF file: {}", path.string());
			return meshData;
		}

		// -- Load binary buffers --
		result = cgltf_load_buffers(&options, data, path.string().c_str());
		if (result != cgltf_result_success) {
			AX_CORE_LOG_ERROR("Failed to load GLTF buffers: {}", path.string());
			cgltf_free(data);
			return meshData;
		}

		for (cgltf_size m = 0; m < data->meshes_count; ++m) {
			const cgltf_mesh& mesh = data->meshes[m];

			for (cgltf_size p = 0; p < mesh.primitives_count; ++p) {
				const cgltf_primitive& primitive = mesh.primitives[p];

				Submesh submesh;
				submesh.baseVertex = static_cast<uint32_t>(meshData.vertices.size());
				submesh.startIndex = static_cast<uint32_t>(meshData.indices.size());

				submesh.materialIndex = 0;
				if (primitive.material) {
					submesh.materialIndex = static_cast<uint32_t>(primitive.material - data->materials);
				}

				// -- Extract Indices --
				if (primitive.indices) {
					cgltf_size indexCount = primitive.indices->count;
					for (cgltf_size i = 0; i < indexCount; ++i) {
						uint32_t index = static_cast<uint32_t>(cgltf_accessor_read_index(primitive.indices, i));
						meshData.indices.push_back(index);
					}
				}

				// -- Extract Vertices --
				cgltf_size vertexCount = 0;
				for (cgltf_size a = 0; a < primitive.attributes_count; ++a) {
					if (primitive.attributes[a].type == cgltf_attribute_type_position) {
						vertexCount = primitive.attributes[a].data->count;
						break;
					}
				}

				std::vector<Vertex> submeshVertices(vertexCount);
				for (cgltf_size a = 0; a < primitive.attributes_count; ++a) {
					const cgltf_attribute& attribute = primitive.attributes[a];

					for (cgltf_size v = 0; v < vertexCount; ++v) {
						float values[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
						cgltf_accessor_read_float(attribute.data, v, values, 4);

						if (attribute.type == cgltf_attribute_type_position) {
							submeshVertices[v].position = { values[0], values[1], values[2] };
						}
						else if (attribute.type == cgltf_attribute_type_normal) {
							submeshVertices[v].normal = { values[0], values[1], values[2] };
						}
						else if (attribute.type == cgltf_attribute_type_texcoord) {
							submeshVertices[v].texcoord = { values[0], values[1] };
						}
						else if (attribute.type == cgltf_attribute_type_tangent) {
							submeshVertices[v].tangent = { values[0], values[1], values[2] };
						}
					}
				}

				meshData.vertices.insert(meshData.vertices.end(), submeshVertices.begin(), submeshVertices.end());

				submesh.indexCount = static_cast<uint32_t>(meshData.indices.size()) - submesh.startIndex;
				if (submesh.indexCount > 0) {
					meshData.submeshes.push_back(submesh);
				}

			}
		}

		cgltf_free(data);
		return meshData;
	}

	DirectX::XMMATRIX getNodeTransform(const cgltf_node* node) {
		if (node->has_matrix) {
			return DirectX::XMMATRIX(node->matrix);
		}

		DirectX::XMVECTOR T = DirectX::XMVectorZero();
		DirectX::XMVECTOR R = DirectX::XMQuaternionIdentity();
		DirectX::XMVECTOR S = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);

		if (node->has_translation) T = DirectX::XMLoadFloat3((DirectX::XMFLOAT3*)node->translation);
		if (node->has_rotation) R = DirectX::XMLoadFloat4((DirectX::XMFLOAT4*)node->rotation);
		if (node->has_scale) S = DirectX::XMLoadFloat3((DirectX::XMFLOAT3*)node->scale);

		return DirectX::XMMatrixAffineTransformation(S, DirectX::XMVectorZero(), R, T);
	}

	SkeletalMeshData GLTFImporter::extractSkeletalMesh(const std::filesystem::path& path) {
		SkeletalMeshData meshData;
		cgltf_options options = {};
		cgltf_data* data = nullptr;

		if (cgltf_parse_file(&options, path.string().c_str(), &data) != cgltf_result_success ||
			cgltf_load_buffers(&options, data, path.string().c_str()) != cgltf_result_success) {
			AX_CORE_LOG_ERROR("Failed to load skeletal GLTF: {}", path.string());
			return meshData;
		}

		if (data->skins_count == 0) {
			cgltf_free(data);
			return meshData;
		}

		// --- 1. Extract Skeleton ---
		const cgltf_skin& skin = data->skins[0]; // Assuming first skin
		std::unordered_map<const cgltf_node*, int> nodeToBoneIndex;
		meshData.skeleton.bones.resize(skin.joints_count);

		// Initialize bones
		for (cgltf_size i = 0; i < skin.joints_count; ++i) {
			cgltf_node* jointNode = skin.joints[i];
			Bone& bone = meshData.skeleton.bones[i];
			bone.name = jointNode->name ? jointNode->name : "Bone_" + std::to_string(i);
			bone.localBindTransform = getNodeTransform(jointNode);
			nodeToBoneIndex[jointNode] = static_cast<int>(i);

			// Read Inverse Bind Matrix
			if (skin.inverse_bind_matrices) {
				float ibm[16];
				cgltf_accessor_read_float(skin.inverse_bind_matrices, i, ibm, 16);
				bone.inverseBindMatrix = DirectX::XMMATRIX(ibm);
			}
			else {
				bone.inverseBindMatrix = DirectX::XMMatrixIdentity();
			}
		}

		// Build Hierarchy
		for (cgltf_size i = 0; i < skin.joints_count; ++i) {
			cgltf_node* jointNode = skin.joints[i];
			if (jointNode->parent && nodeToBoneIndex.count(jointNode->parent)) {
				meshData.skeleton.bones[i].parentIndex = nodeToBoneIndex[jointNode->parent];
				meshData.skeleton.bones[nodeToBoneIndex[jointNode->parent]].children.push_back(i);
			}
		}

		DirectX::XMMATRIX accumulatedTransform = DirectX::XMMatrixIdentity();
		if (skin.joints_count > 0 && skin.joints[0]->parent) {
			cgltf_node* parentNode = skin.joints[0]->parent;
			while (parentNode) {
				// Accumulate transforms from bottom to top
				accumulatedTransform = accumulatedTransform * getNodeTransform(parentNode);
				parentNode = parentNode->parent;
			}
		}
		meshData.skeleton.rootTransform = accumulatedTransform;

		// --- 2. Extract Geometry & Skinning Weights ---
		for (cgltf_size m = 0; m < data->meshes_count; ++m) {
			const cgltf_mesh& mesh = data->meshes[m];
			for (cgltf_size p = 0; p < mesh.primitives_count; ++p) {
				const cgltf_primitive& primitive = mesh.primitives[p];

				Submesh submesh;
				submesh.baseVertex = static_cast<uint32_t>(meshData.vertices.size());
				submesh.startIndex = static_cast<uint32_t>(meshData.indices.size());

				// Indices
				if (primitive.indices) {
					for (cgltf_size i = 0; i < primitive.indices->count; ++i) {
						meshData.indices.push_back(static_cast<uint32_t>(cgltf_accessor_read_index(primitive.indices, i)));
					}
				}

				// Vertices
				cgltf_size vertexCount = primitive.attributes[0].data->count;
				std::vector<SkeletalVertex> submeshVerts(vertexCount);

				for (cgltf_size a = 0; a < primitive.attributes_count; ++a) {
					const cgltf_attribute& attr = primitive.attributes[a];
					for (cgltf_size v = 0; v < vertexCount; ++v) {
						float vals[4] = { 0.0f };
						cgltf_accessor_read_float(attr.data, v, vals, 4);

						if (attr.type == cgltf_attribute_type_position) submeshVerts[v].position = { vals[0], vals[1], vals[2] };
						else if (attr.type == cgltf_attribute_type_normal) submeshVerts[v].normal = { vals[0], vals[1], vals[2] };
						else if (attr.type == cgltf_attribute_type_texcoord) submeshVerts[v].texcoord = { vals[0], vals[1] };
						else if (attr.type == cgltf_attribute_type_tangent) submeshVerts[v].tangent = { vals[0], vals[1], vals[2] };
						else if (attr.type == cgltf_attribute_type_weights) {
							for (int w = 0; w < 4; ++w) submeshVerts[v].boneWeights[w] = vals[w];
						}
						else if (attr.type == cgltf_attribute_type_joints) {
							uint32_t jVals[4] = { 0 };
							cgltf_accessor_read_uint(attr.data, v, jVals, 4);
							for (int j = 0; j < 4; ++j) submeshVerts[v].boneIDs[j] = jVals[j];
						}
					}
				}

				meshData.vertices.insert(meshData.vertices.end(), submeshVerts.begin(), submeshVerts.end());
				submesh.indexCount = static_cast<uint32_t>(meshData.indices.size()) - submesh.startIndex;
				if (submesh.indexCount > 0) meshData.submeshes.push_back(submesh);
			}
		}

		cgltf_free(data);
		return meshData;
	}

	Ref<AnimationClip> GLTFImporter::extractAnimation(const std::filesystem::path& path) {
		cgltf_options options = {};
		cgltf_data* data = nullptr;
		cgltf_parse_file(&options, path.string().c_str(), &data);
		cgltf_load_buffers(&options, data, path.string().c_str());

		if (data->animations_count == 0) {
			cgltf_free(data);
			return nullptr;
		}

		Ref<AnimationClip> clip = std::make_shared<AnimationClip>();
		const cgltf_animation& anim = data->animations[0]; // Loading first animation
		clip->ticksPerSecond = 1.0f; // GLTF uses seconds directly
		clip->duration = 0.0f;

		std::unordered_map<std::string, BoneAnimation> boneAnimMap;

		for (cgltf_size c = 0; c < anim.channels_count; ++c) {
			const cgltf_animation_channel& channel = anim.channels[c];
			const cgltf_animation_sampler& sampler = *channel.sampler;

			std::string nodeName = channel.target_node->name ? channel.target_node->name : "";
			if (nodeName.empty()) continue;

			BoneAnimation& boneAnim = boneAnimMap[nodeName];
			boneAnim.boneName = nodeName;

			cgltf_size keyframeCount = sampler.input->count;
			std::vector<float> times(keyframeCount);
			for (cgltf_size i = 0; i < keyframeCount; ++i) {
				cgltf_accessor_read_float(sampler.input, i, &times[i], 1);
				if (times[i] > clip->duration) clip->duration = times[i];
			}

			for (cgltf_size i = 0; i < keyframeCount; ++i) {
				float vals[4] = { 0.0f };
				cgltf_accessor_read_float(sampler.output, i, vals, 4);

				if (channel.target_path == cgltf_animation_path_type_translation) {
					boneAnim.positions.push_back({ times[i], {vals[0], vals[1], vals[2]} }); // Assuming Vec3 = XMFLOAT3
				}
				else if (channel.target_path == cgltf_animation_path_type_rotation) {
					boneAnim.rotations.push_back({ times[i], {vals[0], vals[1], vals[2], vals[3]} }); // Assuming Vec4 = XMFLOAT4
				}
				else if (channel.target_path == cgltf_animation_path_type_scale) {
					boneAnim.scales.push_back({ times[i], {vals[0], vals[1], vals[2]} });
				}
			}
		}

		for (auto& pair : boneAnimMap) {
			clip->boneAnimations.push_back(pair.second);
		}

		cgltf_free(data);
		return clip;
	}

}
