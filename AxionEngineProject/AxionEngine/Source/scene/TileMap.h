#pragma once

#include "AxionEngine/Source/core/AssetHandle.h"
#include "AxionEngine/Source/core/Timestep.h"
#include "AxionEngine/Source/render/Texture.h"
#include "AxionEngine/Source/render/Camera.h"

namespace Axion {

	class Tile {
	public:

		Tile(AssetHandle<Texture2D> texHandle, bool solid = false)
			: m_textureHandle(texHandle), m_solid(solid) {}

		~Tile() = default;

		AssetHandle<Texture2D> getTextureHandle() const { return m_textureHandle; }
		bool isSolid() const { return m_solid; }

	private:

		AssetHandle<Texture2D> m_textureHandle;
		bool m_solid;

	};



	class TileManager {
	public:

		static void initialize();
		static void shutdown();

		static void registerTile(uint32_t id, AssetHandle<Tile> handle);
		static AssetHandle<Tile> getTileHandle(uint32_t	id);

	private:

		static std::unordered_map<uint32_t, AssetHandle<Tile>> s_idToHandleMap;

	};



	class TileMap {
	public:

		using TileMap2D = std::vector<std::vector<AssetHandle<Tile>>>;

		TileMap();
		TileMap(const std::string& path);
		~TileMap();

		void release();

		void onUpdate(Timestep ts, const Camera& cam);

		bool loadFromFile(const std::string& path);
		bool isLoaded() const { return m_loadedIn; }

	private:

		TileMap2D m_tileMap;
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		bool m_loadedIn = false;

	};

}
