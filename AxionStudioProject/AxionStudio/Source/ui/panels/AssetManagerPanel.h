#pragma once

#include <functional>
#include <string>

#include "AxionEngine/Source/core/AssetManager.h"

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SCollapsingHeader.h"

namespace Axion {

	class AssetManagerPanel {
	public:

		AssetManagerPanel() = default;
		~AssetManagerPanel() = default;

		Silica::WidgetPtr getWidget();

		void refresh();

	private:

		void rebuildUI_Internal();

		std::shared_ptr<Silica::SBox> m_uiRoot;
		bool m_rebuildQueued = false;

	};

}
