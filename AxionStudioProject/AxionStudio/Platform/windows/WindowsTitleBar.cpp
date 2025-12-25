#include "WindowsTitleBar.h"

#if AX_WIN_USING_CUSTOM_TITLE_BAR

#include <Windows.h>

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Platform/windows/WindowsWindow.h"

namespace Axion {

	float WindowsTitleBar::s_lastTitleBarMenuX = 0.0f;

	void WindowsTitleBar::drawCustomTitleBar() {
		s_lastTitleBarMenuX = ImGui::GetCursorPosX();

		// ----- Detect dragging on empty space in the menu bar -----
		static bool draggingWindow = false;
		ImVec2 winPos = ImGui::GetWindowPos();
		ImVec2 cursor = ImGui::GetMousePos();
		ImVec2 local = ImVec2(cursor.x - winPos.x, cursor.y - winPos.y);
		float menuBarHeight = ImGui::GetFrameHeight();
		float buttonWidth = 42.0f;
		float buttonHeight = 24.0f;
		float buttonSpacing = 4.0f;
		float totalButtonWidth = 3 * buttonWidth + 2 * buttonSpacing;
		float dragZoneStartX = s_lastTitleBarMenuX;
		float dragZoneEndX = ImGui::GetWindowWidth() - totalButtonWidth;


		// ----- Callback for the WindowsWindow -----
		if (local.y >= 0 && local.y <= menuBarHeight &&
			local.x >= dragZoneStartX && local.x <= dragZoneEndX) {
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
				draggingWindow = true;
				HWND hwnd = static_cast<HWND>(Application::get().getWindow().getNativeHandle());
				WindowsWindow& win = reinterpret_cast<WindowsWindow&>(Application::get().getWindow());
				win.isDragZone = [menuBarHeight, dragZoneStartX, dragZoneEndX](int x, int y) {
					return y >= 0 && y <= menuBarHeight && x >= dragZoneStartX && x <= dragZoneEndX;
				};
			}
		}
		if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
			draggingWindow = false;
		}


		// ----- Set custom icons for buttons -----
		ImGui::SameLine(ImGui::GetWindowWidth() - 134);
		if (ImGui::Button(u8"\uE15B", { buttonWidth, buttonHeight })) { Application::get().minimizeWindow(); }
		ImGui::SameLine();
		if (ImGui::Button(u8"\uE5D1", { buttonWidth, buttonHeight })) { Application::get().maximizeOrRestoreWindow(); }
		ImGui::SameLine();
		if (ImGui::Button(u8"\uE5CD", { buttonWidth, buttonHeight })) { Application::get().close(); }
	}

}

#endif
