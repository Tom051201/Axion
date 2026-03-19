#pragma once

#include "AxionEngine/Source/core/UUID.h"

#include <filesystem>

namespace Axion {

	enum class AssetType {
		None = 0,
		Mesh,
		Texture2D,
		TextureCube,	
		Material,
		Shader,
		Pipeline,
		Skybox,
		AudioClip,
		PhysicsMaterial,
		Prefab,
		Scene,
	};



	struct AssetMetadata {
		UUID handle;
		AssetType type = AssetType::None;
		std::filesystem::path filePath; // Relative to projects asset folder

		bool isValid() const { return handle.isValid() && type != AssetType::None; }
	};



	class AssetRegistry {
	public:

		AssetRegistry() = default;
		~AssetRegistry() = default;

		void add(const AssetMetadata& metadata);
		void remove(UUID handle);
		bool contains(UUID handle) const;
		const AssetMetadata& get(UUID handle) const;
		void clear();

		const std::unordered_map<UUID, AssetMetadata>& getMap() const { return m_registry; }

		void serialize(const std::string& filepath);
		void deserialize(const std::string& filepath);

		void serializeBinary(const std::string& filepath);
		void deserializeBinary(const std::string& filepath);

		static AssetType assetTypeFromString(const std::string& assetType);
		static std::string assetTypeToString(AssetType type);

	private:

		std::unordered_map<UUID, AssetMetadata> m_registry;

	};

}
