#include "GLTFImporter.h"

#include "AxionEngine/Vendor/cgltf/cgltf.h"

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

#include <fstream>

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

		MeshAssetData meshAssetData;
		meshAssetData.uuid = UUID::generate();
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

		AX_CORE_LOG_TRACE("Extracted Mesh: {}", axmeshPath.filename().string());



		// -- Create Prefab --
		std::filesystem::path prefabOutputDir = ProjectManager::getProject()->getAssetsPath() / "prefabs" / baseName;
		std::filesystem::create_directories(prefabOutputDir);
		AX_CORE_LOG_INFO("Generating GLB Prefab: {}", baseName);

		Ref<Scene> tempScene = std::make_shared<Scene>();
		Entity prefabEntity = tempScene->createEntity(baseName);

		AssetHandle<Mesh> meshHandle;
		meshHandle.uuid = meshAssetData.uuid;
		prefabEntity.addComponent<MeshComponent>(meshHandle);

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

}
