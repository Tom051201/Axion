#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

#include <Silica/include/SWidget.h>

namespace Silica {
	class SBox;
	class SVerticalBox;
	class STextBlock;
	class SButton;
}

namespace Axion {

	class ModalBase {
	public:

		virtual ~ModalBase() = default;

		Silica::WidgetPtr getWidget(std::function<void()> onClose);

	protected:

		virtual void buildContent(std::shared_ptr<Silica::SVerticalBox> contentBox) = 0;
		virtual void validate() = 0;
		virtual void onConfirm() = 0;

		void rebuildUI();

		std::string m_modalTitle = "Modal Title";
		std::string m_confirmText = "Create";
		std::string m_versionText = "";
		float m_modalWidth = 550.0f;

		std::shared_ptr<Silica::STextBlock> m_validationText;
		std::shared_ptr<Silica::SButton> m_confirmBtn;

		Silica::WidgetPtr makeCombo(int& currentIndex, const std::vector<std::string>& options);
		Silica::WidgetPtr makeSliderRow(float& val, float maxVal);
		Silica::WidgetPtr makeFileRow(std::string& outPath, const std::string& typeDesc, const std::string& filter, const std::string& defaultAssetsSubDir);
		Silica::WidgetPtr makeDirectoryRow(std::string& outPath, const std::string& defaultAssetsSubDir);

	private:

		std::shared_ptr<Silica::SBox> m_uiRoot;
		std::function<void()> m_onClose;
		bool m_rebuildQueued = false;

		void rebuildUI_Internal();

	};

}
