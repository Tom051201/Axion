#pragma once

#include <string>
#include <functional>
#include <memory>

#include <Silica/include/SWidget.h>

namespace Silica {
	class SBox;
	class STextBlock;
	class SButton;
}
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
		void validate();

		std::string m_name;
		std::string m_outputPath;

		float m_staticFriction = 0.5f;
		float m_dynamicFriction = 0.5f;
		float m_restitution = 0.05f;

		// -- Silica --
		std::shared_ptr<Silica::SBox> m_uiRoot;
		std::shared_ptr<Silica::STextBlock> m_validationText;
		std::shared_ptr<Silica::SButton> m_createBtn;
		std::function<void()> m_onClose;
		bool m_rebuildQueued = false;

	};

}
