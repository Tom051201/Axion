#pragma once

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"

#include <string>
#include <filesystem>
#include <functional>

namespace Axion {

	class PhysicsMaterialImportModal {
	public:

		PhysicsMaterialImportModal() { resetInputs(); }
		~PhysicsMaterialImportModal() = default;

		Silica::WidgetPtr getWidget(std::function<void()> onClose);

	private:

		void rebuildUI();
		void rebuildUI_Internal();
		void resetInputs();

		std::string m_name;
		std::string m_outputPath;

		float m_staticFriction = 0.5f;
		float m_dynamicFriction = 0.5f;
		float m_restitution = 0.05f;

		// -- Silica --
		std::shared_ptr<Silica::SBox> m_uiRoot;
		std::function<void()> m_onClose;
		bool m_rebuildQueued = false;

	};

}
