#pragma once

#include <memory>
#include <functional>

#include <Silica/include/SWidget.h>

namespace Silica {
	class SDockSpace;
}

namespace Axion {

	class EditorMenuBar {
	public:

		struct MenuBarCallbacks {
			std::function<void()> newScene;
			std::function<void()> openScene;
			std::function<void()> saveScene;
			std::function<void()> saveSceneAs;
			std::function<void()> exitEditor;
			std::function<void()> openPreferences;
		};

		static Silica::WidgetPtr construct(std::shared_ptr<Silica::SDockSpace> dockspace, const MenuBarCallbacks& callbacks);

	};

}
