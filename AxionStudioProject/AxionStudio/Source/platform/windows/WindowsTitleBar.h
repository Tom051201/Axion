#pragma once

#include "AxionEngine/Source/AxionSettings.h"

#if AX_WIN_USING_CUSTOM_TITLE_BAR

#include "AxionEngine/Source/core/Application.h"

namespace Axion {

	class WindowsTitleBar {
	public:

		static void drawCustomTitleBar();

	private:

		static float s_lastTitleBarMenuX;

	};

}

#endif
