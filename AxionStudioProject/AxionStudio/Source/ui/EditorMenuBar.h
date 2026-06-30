#pragma once

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/FontAtlas.h"
#include "AxionStudio/Vendor/Silica/include/SDockSpace.h"

#include <memory>

namespace Axion {

	class EditorMenuBar {
	public:

		static Silica::WidgetPtr construct(Silica::FontAtlas* font, std::shared_ptr<Silica::SDockSpace> dockspace);

	};

}
