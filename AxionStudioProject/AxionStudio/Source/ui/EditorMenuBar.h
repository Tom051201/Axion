#pragma once

#include <memory>

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SDockSpace.h"

namespace Axion {

	class EditorMenuBar {
	public:

		static Silica::WidgetPtr construct(std::shared_ptr<Silica::SDockSpace> dockspace);

	};

}
