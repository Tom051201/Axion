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

	void AssetRegistry::serialize(const std::filesystem::path& filepath) {
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Version" << YAML::Value << 1;
		out << YAML::Key << "AssetRegistry" << YAML::Value << YAML::BeginSeq;

		for (const auto& [uuid, metadata] : m_registry) {
			out << YAML::BeginMap;
			out << YAML::Key << "Handle" << YAML::Value << uuid.toString();
			out << YAML::Key << "Type" << YAML::Value << assetTypeToString(metadata.type);
			out << YAML::Key << "FilePath" << YAML::Value << metadata.filePath.generic_string();
			out << YAML::EndMap;
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void AssetRegistry::deserialize(const std::filesystem::path& filepath) {
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

		uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;

		if (version == 1) {
			for (auto asset : registryNode) {
				AssetMetadata metadata;
				metadata.handle = asset["Handle"].as<UUID>();
				metadata.type = assetTypeFromString(asset["Type"].as<std::string>());
				metadata.filePath = asset["FilePath"].as<std::string>();

				m_registry[metadata.handle] = metadata;
			}
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported AssetRegistry version: {}", version);
			return;
		}

		AX_CORE_LOG_INFO("Loaded AssetRegistry with {} assets.", m_registry.size());
	}

	void AssetRegistry::serializeBinary(const std::filesystem::path& filepath) {
		std::filesystem::create_directories(filepath.parent_path());

		std::ofstream out(filepath, std::ios::out | std::ios::binary);
		if (!out.is_open()) {
			AX_CORE_LOG_ERROR("Failed to open file for binary registry serialization: {}", filepath.string());
			return;
		}

		// -- Write Header --
		char magic[4] = { 'A', 'X', 'A', 'R' };
		out.write(magic, 4);

		uint32_t version = 1;
		out.write(reinterpret_cast<const char*>(&version), sizeof(uint32_t));

		// -- Write Asset Count --
		uint32_t count = static_cast<uint32_t>(m_registry.size());
		out.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));

		// -- Write Entries --
		for (auto const& [uuid, metadata] : m_registry) {
			// -- Write UUID --
			out.write(reinterpret_cast<const char*>(&metadata.handle), sizeof(UUID));

			// -- Write Asset Type --
			uint32_t type = static_cast<uint32_t>(metadata.type);
			out.write(reinterpret_cast<const char*>(&type), sizeof(uint32_t));

			// -- Write File Path --
			std::string pathStr = metadata.filePath.generic_string();
			uint32_t pathLength = static_cast<uint32_t>(pathStr.size());
			out.write(reinterpret_cast<const char*>(&pathLength), sizeof(uint32_t));
			out.write(pathStr.data(), pathLength);
		}

		out.close();
		AX_CORE_LOG_TRACE("Serialized binary AssetRegistry to {}", filepath.string());
	}

	void AssetRegistry::deserializeBinary(const std::filesystem::path& filepath) {
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (!in.is_open()) {
			AX_CORE_LOG_ERROR("Failed to open binary AssetRegistry: {}", filepath.string());
			return;
		}

		// -- Validate Magic Header --
		char magic[4];
		in.read(magic, 4);
		if (memcmp(magic, "AXAR", 4) != 0) {
			AX_CORE_LOG_ERROR("Invalid binary registry file: {}", filepath.string());
			return;
		}

		// -- Validate Version --
		uint32_t version;
		in.read(reinterpret_cast<char*>(&version), sizeof(uint32_t));
		if (version != 1) {
			AX_CORE_LOG_ERROR("Unsupported AssetRegistry binary version: {}", version);
			return;
		}

		m_registry.clear();

		// -- Read Asset Count --
		uint32_t count;
		in.read(reinterpret_cast<char*>(&count), sizeof(uint32_t));

		// -- Read Entries --
		for (uint32_t i = 0; i < count; i++) {
			AssetMetadata metadata;

			// -- Read UUID --
			in.read(reinterpret_cast<char*>(&metadata.handle), sizeof(UUID));

			// -- Read Type --
			uint32_t typeInt;
			in.read(reinterpret_cast<char*>(&typeInt), sizeof(uint32_t));
			metadata.type = static_cast<AssetType>(typeInt);

			// -- Read File Path --
			uint32_t pathLength;
			in.read(reinterpret_cast<char*>(&pathLength), sizeof(uint32_t));
			std::string pathStr(pathLength, '\0');
			in.read(&pathStr[0], pathLength);
			metadata.filePath = pathStr;

			m_registry[metadata.handle] = metadata;
		}

		in.close();
		AX_CORE_LOG_INFO("Loaded Binary AssetRegistry with {} assets.", m_registry.size());
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
		if (assetType == "SkeletalMesh")		return AssetType::SkeletalMesh;
		if (assetType == "AnimationClip")		return AssetType::AnimationClip;
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
			case AssetType::SkeletalMesh:		return "SkeletalMesh";
			case AssetType::AnimationClip:		return "AnimationClip";
		}
		return "None";
	}

}
