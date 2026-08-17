#pragma once

#ifdef AX_PLATFORM_WINDOWS

namespace Axion {

	class WindowsTitleBar {
	public:

		static void drawCustomTitleBar();

	private:

		static float s_lastTitleBarMenuX;

	};

}

#endif
