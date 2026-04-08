#include "AxMaterial.h"

#include "AxionAssetPipeline/Source/core/BaseIncludes.h"

namespace Axion::AAP {

	void MaterialParser::createTextFile(const MaterialAssetData& data, const std::filesystem::path& outputPath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Version" << YAML::Value << ASSET_VERSION_MATERIAL;
		out << YAML::Key << "Name" << YAML::Value << data.name;
		out << YAML::Key << "UUID" << YAML::Value << data.uuid.toString();
		out << YAML::Key << "Type" << YAML::Value << "Material";

		out << YAML::Key << "AlbedoColor" << YAML::Value << data.properties.albedoColor;
		out << YAML::Key << "Metalness" << YAML::Value << data.properties.metalness;
		out << YAML::Key << "Roughness" << YAML::Value << data.properties.roughness;
		out << YAML::Key << "Emission" << YAML::Value << data.properties.emissionStrength;
		out << YAML::Key << "Tiling" << YAML::Value << data.properties.tiling;

		out << YAML::Key << "UseNormalMap" << YAML::Value << data.properties.useNormalMap;
		out << YAML::Key << "UseMetalnessMap" << YAML::Value << data.properties.useMetalnessMap;
		out << YAML::Key << "UseRoughnessMap" << YAML::Value << data.properties.useRoughnessMap;
		out << YAML::Key << "UseOcclusionMap" << YAML::Value << data.properties.useOcclusionMap;
		out << YAML::Key << "UseEmissiveMap" << YAML::Value << data.properties.useEmissiveMap;

		out << YAML::Key << "Textures" << YAML::BeginMap;
		for (auto const& [slot, path] : data.textures) {
			std::string label;
			switch (slot) {
				case TextureSlot::Albedo: { label = "Albedo"; break; }
				case TextureSlot::Normal: { label = "Normal"; break; }
				case TextureSlot::Metalness: { label = "Metalness"; break; }
				case TextureSlot::Roughness: { label = "Roughness"; break; }
				case TextureSlot::Occlusion: { label = "Occlusion"; break; }
				case TextureSlot::Emissive: { label = "Emissive"; break; }
			}

			if (!label.empty()) {
				UUID textureUUID = AssetManager::getAssetUUID(AssetManager::getAbsolute(path));
				out << YAML::Key << label << YAML::Value << textureUUID.toString();
			}
		}
		out << YAML::EndMap;

		if (!data.pipelineAsset.empty()) {
			UUID pipelineUUID = AssetManager::getAssetUUID(AssetManager::getAbsolute(data.pipelineAsset));
			out << YAML::Key << "Pipeline" << YAML::Value << pipelineUUID.toString();
		}

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
		AX_CORE_LOG_TRACE("Created .axmat file ({})", outputPath.string());
	}

	void MaterialParser::createBinaryFile(const MaterialAssetData& data, const std::filesystem::path& outputPath) {
		std::ofstream out(outputPath, std::ios::out | std::ios::binary);
		if (!out) {
			AX_CORE_LOG_ERROR("Failed to create binary file: {}", outputPath.string());
			return;
		}

		// -- Write Header --
		MaterialBinaryHeader header = {};
		header.assetHeader.type = AssetType::Material;
		header.assetHeader.uuid = data.uuid;
		header.assetHeader.version = ASSET_VERSION_MATERIAL;
		header.properties = data.properties;
		if (!data.pipelineAsset.empty()) {
			header.pipelineUUID = AssetManager::getAssetUUID(AssetManager::getAbsolute(data.pipelineAsset));
		}
		else {
			header.pipelineUUID = UUID(0, 0);
		}
		header.textureCount = static_cast<uint32_t>(data.textures.size());
		out.write(reinterpret_cast<const char*>(&header), sizeof(MaterialBinaryHeader));

		// -- Write Textures --
		for (const auto& [slot, path] : data.textures) {
			uint32_t slotInt = static_cast<uint32_t>(slot);
			out.write(reinterpret_cast<const char*>(&slotInt), sizeof(uint32_t));

			UUID textureUUID = AssetManager::getAssetUUID(AssetManager::getAbsolute(path));
			out.write(reinterpret_cast<const char*>(&textureUUID), sizeof(UUID));
		}

		out.close();
		AX_CORE_LOG_TRACE("Baked binary Material to {}", outputPath.string());
	}

}
