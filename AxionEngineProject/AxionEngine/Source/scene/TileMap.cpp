#include "axpch.h"
#include "TileMap.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/core/Application.h"
#include "AxionEngine/Source/render/Renderer2D.h"

namespace Axion {

	std::unordered_map<uint32_t, AssetHandle<Tile>> TileManager::s_idToHandleMap;

	void TileManager::initialize() {

	}

	void TileManager::shutdown() {
		s_idToHandleMap.clear();
	}

	void TileManager::registerTile(uint32_t id, AssetHandle<Tile> handle) {
		s_idToHandleMap[id] = handle;
	}

	AssetHandle<Tile> TileManager::getTileHandle(uint32_t id) {
		return s_idToHandleMap[id];
	}



	TileMap::TileMap() {

	}

	TileMap::TileMap(const std::string& path) {
		loadFromFile(path);
	}

	TileMap::~TileMap() {
		release();
	}

	void TileMap::release() {
		m_loadedIn = false;
	}

	void TileMap::onUpdate(Timestep ts, const Camera& cam) {
		const float TILE_SIZE = 64.0f;
		const float HALF_TILE = TILE_SIZE / 2.0f;

		for (size_t y = 0; y < m_tileMap.size(); y++) {
			for (size_t x = 0; x < m_tileMap[y].size(); x++) {

				AssetHandle<Tile> tileHandle = m_tileMap[y][x];
				if (!tileHandle.isValid()) {
					continue;
				}

				Ref<Tile> tile = AssetManager::get<Tile>(tileHandle);
				AssetHandle<Texture2D> textureHandle = tile->getTextureHandle();
				if (!textureHandle.isValid()) {
					continue;
				}

				Ref<Texture2D> texture = AssetManager::get<Texture2D>(textureHandle);

				// -- World position of tile --
				float worldX = (x * TILE_SIZE) - HALF_TILE;
				float worldY = (y * TILE_SIZE) - HALF_TILE;

				// -- World -> screen --
				Vec2 position = {
					worldX - cam.getViewMatrix().getTranslation().x,
					worldY - cam.getViewMatrix().getTranslation().y,
				};

				//Renderer2D::drawQuad(position, )
				// TODO
			}
		}
	}

	bool TileMap::loadFromFile(const std::string& path) {
		// -- Open file --
		std::ifstream file(path);
		if (!file.is_open()) {
			AX_CORE_LOG_ERROR("Failed to open map file: {}", path);
			return false;
		}

		// -- Reset values --
		m_tileMap.clear();

		std::string line;
		while (std::getline(file, line)) {
			std::stringstream ss(line);
			std::vector<AssetHandle<Tile>> row;
			uint32_t value;

			while (ss >> value) {
				AssetHandle<Tile> tileHandle = TileManager::getTileHandle(value);
				row.push_back(tileHandle);
			}

			if (!row.empty()) {
				m_tileMap.push_back(row);
			}
		}

		file.close();
		return true;
	}

}
