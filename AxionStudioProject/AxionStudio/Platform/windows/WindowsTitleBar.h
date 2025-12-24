#pragma once

#include "AxionEngine/Source/core/Application.h"

namespace Axion {

	class WindowsTitleBar {
	public:

		static void drawCustomTitleBar();

	private:

		static float s_lastTitleBarMenuX;

	};

}
