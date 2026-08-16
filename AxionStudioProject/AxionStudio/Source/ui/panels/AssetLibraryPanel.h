#pragma once

#include <filesystem>
#include <vector>
#include <string>

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"

#include "AxionStudio/Source/core/SilicaContext.h"

#include "AxionEngine/Source/render/Texture.h"

namespace Axion {

	struct AssetPack {
		std::string name;
		std::string description;
		std::filesystem::path sourcePath;
		Silica::TextureID thumbnailID = 0;
		Ref<Texture2D> thumbnailTexture = nullptr;
	};

	class AssetLibraryPanel {
	public:

		AssetLibraryPanel();
		~AssetLibraryPanel() = default;

		Silica::WidgetPtr getWidget();
		void rebuildUI();

		void addLibraryDirectory(const std::filesystem::path& path);
		void setLibraryDirectories(const std::vector<std::filesystem::path>& paths);
		const std::vector<std::filesystem::path>& getLibraryDirectories() const { return m_libraryPaths; }

		void onEvent(Event& e);

	private:

		std::shared_ptr<Silica::SBox> m_uiRoot;
		bool m_rebuildQueued = false;
		bool m_projectIsLoaded = false;

		std::vector<std::filesystem::path> m_libraryPaths;
		std::vector<AssetPack> m_availablePacks;

		std::string m_searchQuery = "";
		std::shared_ptr<Silica::SBox> m_gridContainer;
		std::shared_ptr<Silica::STextBlock> m_packCountText;

		void scanLibraries();
		void rebuildUI_Internal();
		void importAssetPackToProject(const AssetPack& pack);

	};

}
