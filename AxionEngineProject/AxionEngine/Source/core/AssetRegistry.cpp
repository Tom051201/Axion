#include "axpch.h"
#include "AssetRegistry.h"

#include "AxionEngine/Source/core/YamlHelper.h"

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

namespace Axion {

	void AssetRegistry::add(const AssetMetadata& metadata) {
		m_registry[metadata.handle] = metadata;
	}

	void AssetRegistry::remove(UUID handle) {
		m_registry.erase(handle);
	}

	bool AssetRegistry::contains(UUID handle) const {
		return m_registry.find(handle) != m_registry.end();
	}

	const AssetMetadata& AssetRegistry::get(UUID handle) const {
		AX_CORE_ASSERT(contains(handle), "Asset handle not found in registry!");
		return m_registry.at(handle);
	}

	void AssetRegistry::clear() {
		m_registry.clear();
	}

	void AssetRegistry::serialize(const std::string& filepath) {
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "AssetRegistry" << YAML::Value << YAML::BeginSeq;

		for (const auto& [uuid, metadata] : m_registry) {
			out << YAML::BeginMap;
			out << YAML::Key << "Handle" << YAML::Value << uuid.toString();
			out << YAML::Key << "Type" << YAML::Value << assetTypeToString(metadata.type);
			out << YAML::Key << "FilePath" << YAML::Value << metadata.filePath.string();
			out << YAML::EndMap;
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void AssetRegistry::deserialize(const std::string& filepath) {
		m_registry.clear();

		std::ifstream stream(filepath);
		if (!stream.is_open()) return;

		YAML::Node data;
		try {
			data = YAML::Load(stream);
		}
		catch (YAML::ParserException& e) {
			AX_CORE_LOG_ERROR("Failed to parse Asset Registry YAML: {}", e.what());
			return;
		}

		auto registryNode = data["AssetRegistry"];
		if (!registryNode) return;

		for (auto asset : registryNode) {
			AssetMetadata metadata;
			metadata.handle = asset["Handle"].as<UUID>();
			metadata.type = assetTypeFromString(asset["Type"].as<std::string>());
			metadata.filePath = asset["FilePath"].as<std::string>();

			m_registry[metadata.handle] = metadata;
		}

		AX_CORE_LOG_INFO("Loaded AssetRegistry with {} assets.", m_registry.size());
	}

	AssetType AssetRegistry::assetTypeFromString(const std::string& assetType) {
		if (assetType == "Mesh")				return AssetType::Mesh;
		if (assetType == "Texture2D")			return AssetType::Texture2D;
		if (assetType == "TextureCube")			return AssetType::TextureCube;
		if (assetType == "Material")			return AssetType::Material;
		if (assetType == "Shader")				return AssetType::Shader;
		if (assetType == "Pipeline")			return AssetType::Pipeline;
		if (assetType == "Skybox")				return AssetType::Skybox;
		if (assetType == "AudioClip")			return AssetType::AudioClip;
		if (assetType == "PhysicsMaterial")		return AssetType::PhysicsMaterial;
		if (assetType == "Prefab")				return AssetType::Prefab;
		if (assetType == "Scene")				return AssetType::Scene;
		return AssetType::None;
	}

	std::string AssetRegistry::assetTypeToString(AssetType type) {
		switch (type) {
			case AssetType::Mesh:				return "Mesh";
			case AssetType::Texture2D:			return "Texture2D";
			case AssetType::TextureCube:		return "TextureCube";
			case AssetType::Material:			return "Material";
			case AssetType::Shader:				return "Shader";
			case AssetType::Pipeline:			return "Pipeline";
			case AssetType::Skybox:				return "Skybox";
			case AssetType::AudioClip:			return "AudioClip";
			case AssetType::PhysicsMaterial:	return "PhysicsMaterial";
			case AssetType::Prefab:				return "Prefab";
			case AssetType::Scene:				return "Scene";
			case AssetType::None:				return "None";
		}
		return "None";
	}

}
