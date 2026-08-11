#pragma once

#include <filesystem>
#include <vector>

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"

namespace Axion {

	class AssetLibraryPanel {
	public:

		AssetLibraryPanel();
		~AssetLibraryPanel() = default;

		Silica::WidgetPtr getWidget();
		void rebuildUI();

	private:

		std::shared_ptr<Silica::SBox> m_uiRoot;
		bool m_rebuildQueued = false;

		std::filesystem::path m_engineDefaultAssetsPath;

		void rebuildUI_Internal();
		void importAssetToProject(const std::filesystem::path& sourceAssetPath);

	};

}
