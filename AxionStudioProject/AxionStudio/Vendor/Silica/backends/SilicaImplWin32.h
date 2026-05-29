#pragma once

#include <Windows.h>

#include "../include/SWidget.h"

namespace Silica {

	bool ImplWin32_init(HWND hwnd);

	void ImplWin32_shutdown();

	bool ImplWin32_wndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, WidgetPtr rootWidget);

}
