#pragma once

#include <functional>

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"

namespace Axion {

	class SettingsModal {
	public:

		SettingsModal() = default;
		~SettingsModal() = default;

		Silica::WidgetPtr getWidget(std::function<void()> onClose);

	private:

		void rebuildUI();
		void rebuildUI_Internal();

		// -- Silica --
		std::shared_ptr<Silica::SBox> m_uiRoot;
		std::function<void()> m_onClose;
		bool m_rebuildQueued = false;

		std::string m_budgetText;

	};

}
