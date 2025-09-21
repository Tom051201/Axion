#pragma once

#include "AxionStudio/Source/core/Panel.h"

namespace Axion {

	class AssetManagerPanel : public Panel {
	public:

		AssetManagerPanel(const std::string& name);
		~AssetManagerPanel() override;

		void setup() override;
		void shutdown() override;
		void onGuiRender() override;

	};


}
