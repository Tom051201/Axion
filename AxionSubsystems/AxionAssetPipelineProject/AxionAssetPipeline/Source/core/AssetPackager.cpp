#include "AssetPackager.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/EnumUtils.h"
#include "AxionEngine/Source/core/YAMLHelper.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionAssetPipeline/Source/AxMesh.h"
#include "AxionAssetPipeline/Source/AxMaterial.h"
#include "AxionAssetPipeline/Source/AxPhysicsMaterial.h"
#include "AxionAssetPipeline/Source/AxPipeline.h"
#include "AxionAssetPipeline/Source/AxShader.h"
#include "AxionAssetPipeline/Source/AxSkybox.h"
#include "AxionAssetPipeline/Source/AxTextureCube.h"
#include "AxionAssetPipeline/Source/AxTexture2D.h"

namespace Axion::AAP {

	void AssetPackager::packageProject(const std::string& outputDirectory) {
		AX_CORE_LOG_INFO("Starting Asset Packaging to: {}", outputDirectory);

		std::filesystem::create_directories(outputDirectory);

		auto project = ProjectManager::getProject();
		if (!project) {
			AX_CORE_LOG_ERROR("Cannot package: No active Project.");
			return;
		}

		Ref<AssetRegistry> inRegistry = project->getAssetRegistry();
		AssetRegistry runtimeRegistry;

		for (const auto& [uuid, metadata] : inRegistry->getMap()) {
			std::filesystem::path inPath = AssetManager::getAbsolute(metadata.filePath.string());
			std::filesystem::path runtimeRelativePath = getRuntimePath(metadata.filePath);
			std::filesystem::path runtimeAbsolutePath = std::filesystem::path(outputDirectory) / runtimeRelativePath;

			std::filesystem::create_directories(runtimeAbsolutePath.parent_path());

			switch (metadata.type) {
				case AssetType::Mesh: {
					std::ifstream stream(inPath);
					YAML::Node data = YAML::Load(stream);

					MeshAssetData meshData;
					meshData.uuid = uuid;
					meshData.name = data["Name"].as<std::string>();
					meshData.fileFormat = data["Format"].as<std::string>();
					meshData.filePath = AssetManager::getAbsolute(data["Source"].as<std::string>());

					MeshParser::createBinaryFile(meshData, runtimeAbsolutePath.string());
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
					
					UUID pipeUUID = UUID::fromString(data["Pipeline"].as<std::string>());
					auto registry = ProjectManager::getProject()->getAssetRegistry();
					if (registry->contains(pipeUUID)) {
						matData.pipelineAsset = registry->get(pipeUUID).filePath.string();
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

					MaterialParser::createBinaryFile(matData, runtimeAbsolutePath.string());
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

					PhysicsMaterialParser::createBinaryFile(pmatData, runtimeAbsolutePath.string());
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

					PipelineParser::createBinaryFile(pipeData, runtimeAbsolutePath.string());
					break;
				}
				case AssetType::Shader: {
					std::ifstream stream(inPath);
					YAML::Node data = YAML::Load(stream);

					ShaderAssetData shaderData;
					shaderData.uuid = uuid;
					shaderData.fileFormat = data["Format"].as<std::string>();
					shaderData.filePath = AssetManager::getAbsolute(data["Source"].as<std::string>());

					YAML::Node specData = data["Specification"];
					shaderData.spec.name = specData["Name"].as<std::string>();
					if (specData["BatchTextures"]) {
						shaderData.spec.batchTextures = specData["BatchTextures"].as<uint32_t>();
					}

					ShaderParser::createBinaryFile(shaderData, runtimeAbsolutePath.string());
					break;
				}
				case AssetType::Skybox: {
					std::ifstream stream(inPath);
					YAML::Node data = YAML::Load(stream);

					SkyboxAssetData skyData;
					skyData.uuid = uuid;
					skyData.name = data["Name"].as<std::string>();

					auto registry = ProjectManager::getProject()->getAssetRegistry();
					UUID pipeUUID = UUID::fromString(data["Pipeline"].as<std::string>());
					if (registry->contains(pipeUUID)) skyData.pipelinePath = registry->get(pipeUUID).filePath.string();

					UUID texUUID = UUID::fromString(data["TextureCube"].as<std::string>());
					if (registry->contains(texUUID)) skyData.textureCubePath = registry->get(texUUID).filePath.string();

					SkyboxParser::createBinaryFile(skyData, runtimeAbsolutePath.string());
					break;
				}
				case AssetType::TextureCube: {
					std::ifstream stream(inPath);
					YAML::Node data = YAML::Load(stream);

					TextureCubeAssetData texData;
					texData.uuid = uuid;
					texData.name = data["Name"].as<std::string>();
					texData.fileFormat = data["Format"].as<std::string>();
					texData.filePath = AssetManager::getAbsolute(data["Source"].as<std::string>());

					TextureCubeParser::createBinaryFile(texData, runtimeAbsolutePath.string());
					break;
				}
				case AssetType::Texture2D: {
					std::ifstream stream(inPath);
					YAML::Node data = YAML::Load(stream);

					Texture2DAssetData texData;
					texData.uuid = uuid;
					texData.name = data["Name"].as<std::string>();
					texData.fileFormat = data["Format"].as<std::string>();
					texData.filePath = AssetManager::getAbsolute(data["Source"].as<std::string>());

					Texture2DParser::createBinaryFile(texData, runtimeAbsolutePath.string());
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

		std::string registryOutputPath = (std::filesystem::path(outputDirectory) / "AssetRegistry.bin").string();
		runtimeRegistry.serializeBinary(registryOutputPath);

		AX_CORE_LOG_INFO("Asset Packaging Complete!");
	}

	std::filesystem::path AssetPackager::getRuntimePath(const std::filesystem::path& inPath) {
		std::filesystem::path newPath = inPath;
		newPath.replace_extension(".axbin");
		return newPath;
	}

}
