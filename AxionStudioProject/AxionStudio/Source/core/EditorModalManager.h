#pragma once

#include <memory>

#include <Silica/include/SWidget.h> 
#include <Silica/include/OverlayManager.h>

namespace Silica {
	class SBox;
}

namespace Axion {

	class EditorModalManager {
	public:

		static void shutdown();
		static void open(Silica::WidgetPtr modalWidget);
		static void close();

		static bool isModalOpen() { return s_activeModalID != 0; }

	private:

		inline static Silica::OverlayID s_activeModalID = 0;

	};

}
