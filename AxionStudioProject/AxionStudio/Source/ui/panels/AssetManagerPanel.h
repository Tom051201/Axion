#pragma once

#include "AxionEngine/Source/core/AssetManager.h"

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SCollapsingHeader.h"
#include "AxionStudio/Vendor/Silica/include/FontAtlas.h"

#include <functional>
#include <string>

namespace Axion {

	class AssetManagerPanel {
	public:

		AssetManagerPanel() = default;
		~AssetManagerPanel() = default;

		Silica::WidgetPtr getWidget(Silica::FontAtlas* font);

		void refresh();

	private:

		void rebuildUI_Internal();

		std::shared_ptr<Silica::SBox> m_uiRoot;
		Silica::FontAtlas* m_font = nullptr;
		bool m_rebuildQueued = false;

	};

}
