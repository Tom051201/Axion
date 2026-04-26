#include "AssetPackager.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/EnumUtils.h"
#include "AxionEngine/Source/core/YAMLHelper.h"
#include "AxionEngine/Source/scene/SceneSerializer.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionAssetPipeline/Source/platform/PlatformPackager.h"
#include "AxionAssetPipeline/Source/AxMesh.h"
#include "AxionAssetPipeline/Source/AxMaterial.h"
#include "AxionAssetPipeline/Source/AxPhysicsMaterial.h"
#include "AxionAssetPipeline/Source/AxPipeline.h"
#include "AxionAssetPipeline/Source/AxShader.h"
#include "AxionAssetPipeline/Source/AxSkybox.h"
#include "AxionAssetPipeline/Source/AxTextureCube.h"
#include "AxionAssetPipeline/Source/AxTexture2D.h"
#include "AxionAssetPipeline/Source/AxPrefab.h"
#include "AxionAssetPipeline/Source/AxAudio.h"
#include "AxionAssetPipeline/Source/AxAnimationClip.h"
#include "AxionAssetPipeline/Source/AxSkeletalMesh.h"

namespace Axion::AAP {

	void AssetPackager::packageProject(const std::filesystem::path& outputDirectory) {
		AX_CORE_LOG_INFO("Starting Asset Packaging to: {}", outputDirectory.string());

		std::filesystem::create_directories(outputDirectory);

		auto project = ProjectManager::getProject();
		if (!project) {
			AX_CORE_LOG_ERROR("Cannot package: No active Project.");
			return;
		}

		Ref<AssetRegistry> inRegistry = project->getAssetRegistry();
		AssetRegistry runtimeRegistry;

		for (const auto& [uuid, metadata] : inRegistry->getMap()) {
			std::filesystem::path inPath = AssetManager::getAbsolute(metadata.filePath);
			std::filesystem::path runtimeRelativePath = getRuntimePath(metadata.filePath);
			std::filesystem::path runtimeAbsolutePath = outputDirectory / runtimeRelativePath;

			std::filesystem::create_directories(runtimeAbsolutePath.parent_path());

			switch (metadata.type) {
				case AssetType::Mesh: {
					std::ifstream stream(inPath);
					YAML::Node data = YAML::Load(stream);

					MeshAssetData meshData;
					meshData.uuid = uuid;
					meshData.name = data["Name"].as<std::string>();
					meshData.fileFormat = FormatUtils::meshFormatFromString(data["Format"].as<std::string>());
					meshData.filePath = AssetManager::getAbsolute(data["Source"].as<std::string>());

					MeshParser::createBinaryFile(meshData, runtimeAbsolutePath);
					break;
				}
				case AssetType::AudioClip: {
					std::ifstream stream(inPath);
					YAML::Node data = YAML::Load(stream);

					AudioAssetData audioData;
					audioData.uuid = uuid;
					audioData.name = data["Name"].as<std::string>();
					audioData.audioFilePath = AssetManager::getAbsolute(data["Source"].as<std::string>());
					audioData.fileFormat = FormatUtils::audioFormatFromString(data["Format"].as<std::string>());
					audioData.mode = EnumUtils::AudioClipModeFromString(data["Mode"].as<std::string>());

					AudioParser::createBinaryFile(audioData, runtimeAbsolutePath);
					break;
				}
				case AssetType::Material: {
					std::ifstream stream(inPath);
					YAML::Node data = YAML::Load(stream);

					MaterialAssetData matData;
					matData.uuid = uuid;
					matData.name = data["Name"].as<std::string>();
					matData.properties.albedoColor = data["AlbedoColor"].as<Vec4>();
					matData.properties.metalness = data["Metalness"].as<float>();
					matData.properties.roughness = data["Roughness"].as<float>();
					matData.properties.emissionStrength = data["Emission"].as<float>();
					matData.properties.tiling = data["Tiling"].as<float>();
					matData.properties.useNormalMap = data["UseNormalMap"].as<float>();
					matData.properties.useMetalnessMap = data["UseMetalnessMap"].as<float>();
					matData.properties.useRoughnessMap = data["UseRoughnessMap"].as<float>();
					matData.properties.useOcclusionMap = data["UseOcclusionMap"].as<float>();
					
					auto registry = ProjectManager::getProject()->getAssetRegistry();

					if (data["Pipeline"]) {
						UUID pipeUUID = UUID::fromString(data["Pipeline"].as<std::string>());
						if (registry->contains(pipeUUID)) {
							matData.pipelineAsset = registry->get(pipeUUID).filePath.string();
						}
					}
					else {
						matData.pipelineAsset = "";
					}

					if (data["Textures"]) {
						auto texturesNode = data["Textures"];
						if (texturesNode["Albedo"]) matData.textures[TextureSlot::Albedo] = registry->get(UUID::fromString(texturesNode["Albedo"].as<std::string>())).filePath.string();
						if (texturesNode["Normal"]) matData.textures[TextureSlot::Normal] = registry->get(UUID::fromString(texturesNode["Normal"].as<std::string>())).filePath.string();
						if (texturesNode["Metalness"]) matData.textures[TextureSlot::Metalness] = registry->get(UUID::fromString(texturesNode["Metalness"].as<std::string>())).filePath.string();
						if (texturesNode["Roughness"]) matData.textures[TextureSlot::Roughness] = registry->get(UUID::fromString(texturesNode["Roughness"].as<std::string>())).filePath.string();
						if (texturesNode["Occlusion"]) matData.textures[TextureSlot::Occlusion] = registry->get(UUID::fromString(texturesNode["Occlusion"].as<std::string>())).filePath.string();
						if (texturesNode["Emissive"]) matData.textures[TextureSlot::Emissive] = registry->get(UUID::fromString(texturesNode["Emissive"].as<std::string>())).filePath.string();
					}

					MaterialParser::createBinaryFile(matData, runtimeAbsolutePath);
					break;
				}
				case AssetType::PhysicsMaterial: {
					std::ifstream stream(inPath);
					YAML::Node data = YAML::Load(stream);

					PhysicsMaterialAssetData pmatData;
					pmatData.uuid = uuid;
					pmatData.name = data["Name"].as<std::string>();
					pmatData.staticFriction = data["StaticFriction"].as<float>();
					pmatData.dynamicFriction = data["DynamicFriction"].as<float>();
					pmatData.restitution = data["Restitution"].as<float>();

					PhysicsMaterialParser::createBinaryFile(pmatData, runtimeAbsolutePath);
					break;
				}
				case AssetType::Pipeline: {
					std::ifstream stream(inPath);
					YAML::Node data = YAML::Load(stream);

					PipelineAssetData pipeData;
					pipeData.uuid = uuid;
					pipeData.name = data["Name"].as<std::string>();

					YAML::Node specData = data["Specification"];

					UUID shaderUUID = UUID::fromString(specData["Shader"].as<std::string>());
					auto registry = ProjectManager::getProject()->getAssetRegistry();
					if (registry->contains(shaderUUID)) {
						pipeData.shaderFilePath = registry->get(shaderUUID).filePath.string();
					}

					pipeData.spec.numRenderTargets = specData["NumRenderTargets"].as<uint32_t>();
					pipeData.spec.colorFormat = EnumUtils::colorFormatFromString(specData["ColorFormat"].as<std::string>());
					pipeData.spec.depthStencilFormat = EnumUtils::depthStencilFormatFromString(specData["DepthStencilFormat"].as<std::string>());
					pipeData.spec.depthTest = specData["DepthTest"].as<bool>();
					pipeData.spec.depthWrite = specData["DepthWrite"].as<bool>();
					pipeData.spec.depthFunction = EnumUtils::depthCompareFromString(specData["DepthFunction"].as<std::string>());
					pipeData.spec.stencilEnabled = specData["StencilEnabled"].as<bool>();
					pipeData.spec.sampleCount = specData["SampleCount"].as<uint32_t>();
					pipeData.spec.cullMode = EnumUtils::cullModeFromString(specData["CullMode"].as<std::string>());
					pipeData.spec.topology = EnumUtils::primitiveTopologyFromString(specData["Topology"].as<std::string>());

					YAML::Node layoutData = specData["BufferLayout"];
					if (layoutData && layoutData.IsSequence()) {
						std::vector<BufferElement> elements;
						elements.reserve(layoutData.size());
						for (const auto& elemNode : layoutData) {
							std::string name = elemNode["Name"].as<std::string>();
							ShaderDataType type = EnumUtils::shaderDataTypeFromString(elemNode["Type"].as<std::string>());
							BufferElement elem(name, type);
							if (elemNode["Size"]) elem.size = elemNode["Size"].as<uint32_t>();
							if (elemNode["Offset"]) elem.offset = elemNode["Offset"].as<uint32_t>();
							if (elemNode["Instanced"]) elem.instanced = elemNode["Instanced"].as<bool>();
							elements.push_back(elem);
						}
						BufferLayout layout(elements);
						layout.calculateOffsetAndStride();
						pipeData.spec.vertexLayout = layout;
					}

					PipelineParser::createBinaryFile(pipeData, runtimeAbsolutePath);
					break;
				}
				case AssetType::Shader: {
					std::ifstream stream(inPath);
					YAML::Node data = YAML::Load(stream);

					ShaderAssetData shaderData;
					shaderData.uuid = uuid;
					shaderData.fileFormat = FormatUtils::shaderFormatFromString(data["Format"].as<std::string>());
					shaderData.filePath = AssetManager::getAbsolute(data["Source"].as<std::string>());

					YAML::Node specData = data["Specification"];
					shaderData.spec.name = specData["Name"].as<std::string>();
					if (specData["BatchTextures"]) {
						shaderData.spec.batchTextures = specData["BatchTextures"].as<uint32_t>();
					}

					ShaderParser::createBinaryFile(shaderData, runtimeAbsolutePath);
					break;
				}
				case AssetType::Skybox: {
					std::ifstream stream(inPath);
					YAML::Node data = YAML::Load(stream);

					SkyboxAssetData skyData;
					skyData.uuid = uuid;
					skyData.name = data["Name"].as<std::string>();

					auto registry = ProjectManager::getProject()->getAssetRegistry();

					if (data["Pipeline"]) {
						UUID pipeUUID = UUID::fromString(data["Pipeline"].as<std::string>());
						if (registry->contains(pipeUUID)) {
							skyData.pipelinePath = registry->get(pipeUUID).filePath.string();
						}
					}
					else {
						skyData.pipelinePath = "";
					}

					UUID texUUID = UUID::fromString(data["TextureCube"].as<std::string>());
					if (registry->contains(texUUID)) skyData.textureCubePath = registry->get(texUUID).filePath.string();

					SkyboxParser::createBinaryFile(skyData, runtimeAbsolutePath);
					break;
				}
				case AssetType::TextureCube: {
					std::ifstream stream(inPath);
					YAML::Node data = YAML::Load(stream);

					TextureCubeAssetData texData;
					texData.uuid = uuid;
					texData.name = data["Name"].as<std::string>();
					texData.fileFormat = FormatUtils::textureFormatFromString(data["Format"].as<std::string>());
					texData.filePath = AssetManager::getAbsolute(data["Source"].as<std::string>());

					TextureCubeParser::createBinaryFile(texData, runtimeAbsolutePath);
					break;
				}
				case AssetType::Texture2D: {
					std::ifstream stream(inPath);
					YAML::Node data = YAML::Load(stream);

					Texture2DAssetData texData;
					texData.uuid = uuid;
					texData.name = data["Name"].as<std::string>();
					texData.fileFormat = FormatUtils::textureFormatFromString(data["Format"].as<std::string>());
					texData.filePath = AssetManager::getAbsolute(data["Source"].as<std::string>());

					Texture2DParser::createBinaryFile(texData, runtimeAbsolutePath);
					break;
				}
				case AssetType::Prefab: {
					std::ifstream stream(inPath);
					YAML::Node data = YAML::Load(stream);

					PrefabAssetData prefabData;
					prefabData.uuid = uuid;
					prefabData.name = data["Name"] ? data["Name"].as<std::string>() : "Prefab";
					prefabData.scene = std::make_shared<Scene>();

					YAML::Node entityNode = data["Entity"];
					prefabData.entity = SceneSerializer::deserializeEntityNode(prefabData.scene.get(), entityNode, false);

					PrefabParser::createBinaryFile(prefabData, runtimeAbsolutePath);
					break;
				}
				case AssetType::Scene: {
					Ref<Scene> scene = std::make_shared<Scene>();
					SceneSerializer serializer(scene);
					if (serializer.deserializeText(inPath)) {
						serializer.serializeBinary(runtimeAbsolutePath);
					}
					else {
						AX_CORE_LOG_ERROR("Failed to bake scene {}", inPath.string());
					}
					break;
				}
				case AssetType::AnimationClip: {
					std::ifstream stream(inPath);
					YAML::Node data = YAML::Load(stream);

					AnimationClipAssetData clipData;
					clipData.uuid = uuid;
					clipData.name = data["Name"].as<std::string>();
					clipData.filePath = AssetManager::getAbsolute(data["Source"].as<std::string>());

					AnimationClipParser::createBinaryFile(clipData, runtimeAbsolutePath);
					break;
				}
				case AssetType::SkeletalMesh: {
					std::ifstream stream(inPath);
					YAML::Node data = YAML::Load(stream);

					SkeletalMeshAssetData meshData;
					meshData.uuid = uuid;
					meshData.name = data["Name"].as<std::string>();
					meshData.filePath = AssetManager::getAbsolute(data["Source"].as<std::string>());

					SkeletalMeshParser::createBinaryFile(meshData, runtimeAbsolutePath);
					break;
				}
				default: {
					AX_CORE_LOG_WARN("Asset Type not supported for packaging yet: {}", AssetRegistry::assetTypeToString(metadata.type));
					break;
				}
			}

			AssetMetadata runtimeMetadata;
			runtimeMetadata.handle = uuid;
			runtimeMetadata.type = metadata.type;
			runtimeMetadata.filePath = runtimeRelativePath;

			runtimeRegistry.add(runtimeMetadata);
		}

		std::filesystem::path registryOutputPath = outputDirectory / "AssetRegistry.bin";
		runtimeRegistry.serializeBinary(registryOutputPath);

		AX_CORE_LOG_INFO("Asset Packaging Complete!");

		std::filesystem::path configPath = outputDirectory / "GameConfig.axbin";
		std::ofstream outConfig(configPath, std::ios::out | std::ios::binary);

		if (outConfig.is_open()) {
			// -- Write Header --
			char magic[4] = { 'A', 'X', 'C', 'F' };
			outConfig.write(magic, 4);
			uint32_t version = 1; // TODO: create central field
			outConfig.write(reinterpret_cast<const char*>(&version), sizeof(uint32_t));

			// -- Write Game Name --
			std::string projectName = project->getName();
			uint32_t nameLength = static_cast<uint32_t>(projectName.size());
			outConfig.write(reinterpret_cast<const char*>(&nameLength), sizeof(uint32_t));
			outConfig.write(projectName.data(), nameLength);

			// -- Write Default Scene UUID --
			UUID defaultSceneUUID = UUID(0, 0);
			if (!project->getDefaultScene().empty()) {
				std::filesystem::path normalizedPath = AssetManager::getRelativeToAssets(AssetManager::getAbsolute(project->getDefaultScene()));
				for (const auto& [uuid, metadata] : inRegistry->getMap()) {
					if (metadata.filePath == normalizedPath) {
						defaultSceneUUID = uuid;
						break;
					}
				}
			}
			outConfig.write(reinterpret_cast<const char*>(&defaultSceneUUID), sizeof(UUID));

			// -- Write App Icon Path --
			std::string iconPath = project->getAppIconPath().generic_string();
			uint32_t iconPathLength = static_cast<uint32_t>(iconPath.size());
			outConfig.write(reinterpret_cast<const char*>(&iconPathLength), sizeof(uint32_t));
			if (iconPathLength > 0) {
				outConfig.write(iconPath.data(), iconPathLength);
			}

			outConfig.close();
			AX_CORE_LOG_INFO("Baked GameConfig.axbin");

			// -- Copy and Setup the Runtime Executable --
			std::string gameName = project->getName();
			std::filesystem::path exportedExePath = outputDirectory / (gameName + ".exe");
			std::filesystem::path sourceRuntimePath = "RuntimeData/AxionRuntime.exe";

			if (std::filesystem::exists(sourceRuntimePath)) {
				try {
					// -- Copy Runtime Executable --
					std::filesystem::copy_file(sourceRuntimePath, exportedExePath, std::filesystem::copy_options::overwrite_existing);
					AX_CORE_LOG_INFO("Copied Runtime Player to: {}", exportedExePath.string());

					// -- Inject .ico file --
					std::filesystem::path iconPath = project->getAppIconPath();
					if (!iconPath.empty()) {
						std::filesystem::path absIconPath = AssetManager::getAbsolute(iconPath);
						if (PlatformPackager::injectIconIntoExecutable(exportedExePath, absIconPath)) {
							AX_CORE_LOG_INFO("Successfully injected custom executable icon!");
						}
						else {
							AX_CORE_LOG_WARN("Failed to inject executable icon. Ensure the file is a valid .ico format.");
						}
					}

					// -- Copy Dependencies --
					const char* requiredFiles[] = {
						"AxionScriptCore.dll",
						"AxionScriptCore.runtimeconfig.json",
						"freeglutd.dll",
						"nethost.dll",
						"PhysX_64.dll",
						"PhysXCommon_64.dll",
						"PhysXCooking_64.dll",
						"PhysXFoundation_64.dll",
						"PVDRuntime_64.dll"
					};

					for (const char* fileName : requiredFiles) {
						std::filesystem::path sourceFile = std::filesystem::path("RuntimeData") / fileName;
						std::filesystem::path destFile = outputDirectory / fileName;

						if (std::filesystem::exists(sourceFile)) {
							std::filesystem::copy_file(sourceFile, destFile, std::filesystem::copy_options::overwrite_existing);
							AX_CORE_LOG_INFO("Copied dependency: {}", fileName);
						}
						else {
							AX_CORE_LOG_ERROR("Failed to copy {}. Source not found at: {}", fileName, sourceFile.string());
						}
					}

				}
				catch (const std::exception& e) {
					AX_CORE_LOG_ERROR("Failed to copy Runtime Player: {}", e.what());
				}
			}
			else {
				AX_CORE_LOG_ERROR("Could not find the pre-compiled AxionRuntime.exe at {}", sourceRuntimePath.string());
			}
		}

	}

	std::filesystem::path AssetPackager::getRuntimePath(const std::filesystem::path& inPath) {
		std::string stem = inPath.stem().string();
		std::string ext = inPath.extension().string();
		if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);
		std::string newFileName = stem + "_" + ext + ".axbin";
		std::filesystem::path newPath = inPath;
		newPath.replace_filename(newFileName);
		return newPath;
	}

}
