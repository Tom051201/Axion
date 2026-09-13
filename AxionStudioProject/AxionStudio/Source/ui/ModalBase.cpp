#include "studiopch.h"
#include "ModalBase.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SButton.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SAlign.h>
#include <Silica/include/SSeparator.h>
#include <Silica/include/SComboBox.h>
#include <Silica/include/SSliderFloat.h>
#include <Silica/include/SEditableText.h>

#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/core/PlatformUtils.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"

namespace Axion {

	Silica::WidgetPtr ModalBase::getWidget(std::function<void()> onClose) {
		m_onClose = onClose;

		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.consumePointerEvents = true,
				.backgroundColor = Silica::Color(0, 0, 0, 180),
			});
			rebuildUI_Internal();
		}
		return m_uiRoot;
	}

	void ModalBase::rebuildUI() {
		if (m_rebuildQueued) return;
		m_rebuildQueued = true;

		EditorActionQueue::push([this]() {
			m_rebuildQueued = false;
			rebuildUI_Internal();
		});
	}

	void ModalBase::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 12.0f });

		// -- Standard Header --
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = m_modalTitle }) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({}) });

		buildContent(contentBox);

		// -- Standard Footer references --
		m_validationText = Silica::MakeWidget<Silica::STextBlock>({.text = "" });

		m_confirmBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = { 20.0f, 8.0f },
			.onClick = [this]() {
				if (!m_confirmBtn->isEnabled()) return Silica::EventReply::unhandled();

				onConfirm();

				if (m_onClose) m_onClose();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = m_confirmText })
		});

		contentBox->addSlot({ {0,0}, m_validationText });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({}) });

		// -- Standard Footer Assembly --
		auto cancelBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = { 20.0f, 8.0f },
			.onClick = [this]() {
				if (m_onClose) m_onClose();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Cancel" })
		});

		auto footerRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 10.0f,
			.slots = {
				{ {0,0}, m_confirmBtn },
				{ {0,0}, cancelBtn },
				{ {1,0}, Silica::MakeWidget<Silica::SBox>({.backgroundColor = Silica::Color::transparent()}) },
				{ {0,0}, Silica::MakeWidget<Silica::SAlign>({
					.verticalAlign = Silica::VerticalAlign::Center,
					.child = Silica::MakeWidget<Silica::STextBlock>({
						.text = m_versionText,
						.color = Silica::GetTheme().Text_Dim
					})
				})}
			}
		});

		contentBox->addSlot({ {0,0}, footerRow });

		// -- Final Outer Box --
		auto modalPanel = Silica::MakeWidget<Silica::SBox>({
			.explicitSize = Silica::Vec2{ m_modalWidth, 0.0f },
			.borderThickness = Silica::GetTheme().Border_Thickness,
			.backgroundColor = Silica::GetTheme().Background_Panel,
			.child = Silica::MakeWidget<Silica::SBox>({
				.padding = { 20.0f, 20.0f },
				.backgroundColor = Silica::Color::transparent(),
				.child = contentBox
			})
		});

		m_uiRoot->setChild(Silica::MakeWidget<Silica::SAlign>({
			.horizontalAlign = Silica::HorizontalAlign::Center,
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = modalPanel
		}));

		validate();
	}



	// ----- SHARED HELPERS -----
	Silica::WidgetPtr ModalBase::makePropertyRow(const std::string& label, Silica::WidgetPtr valueWidget) {
		return Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 10.0f,
			.slots = {
				{ {0, 0}, Silica::MakeWidget<Silica::SBox>({
					.explicitSize = Silica::Vec2(130.0f, 0.0f),
					.backgroundColor = Silica::Color::transparent(),
					.child = Silica::MakeWidget<Silica::SAlign>({
						.verticalAlign = Silica::VerticalAlign::Center,
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = label })
					})
				})},
				{ {1, 0}, valueWidget }
			}
		});
	}

	Silica::WidgetPtr ModalBase::makeCombo(int& currentIndex, const std::vector<std::string>& options) {
		return Silica::MakeWidget<Silica::SComboBox>({
			.options = options,
			.initialValue = options[currentIndex],
			.onValueChanged = [this, &currentIndex, options](const std::string& selectedVal) {
				auto it = std::find(options.begin(), options.end(), selectedVal);
				if (it != options.end()) {
					currentIndex = static_cast<int>(std::distance(options.begin(), it));
					rebuildUI();
				}
			}
		});
	}

	Silica::WidgetPtr ModalBase::makeSliderRow(float& val, float maxVal) {
		return Silica::MakeWidget<Silica::SSliderFloat>({
			.initialValue = val, .minValue = 0.0f, .maxValue = maxVal,
			.onValueChanged = [this, &val](float v) { val = v; }
		});
	}

	Silica::WidgetPtr ModalBase::makeFileRow(std::string& outPath, const std::string& typeDesc, const std::string& filter, const std::string& defaultAssetsSubDir) {
		return Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 8.0f,
			.slots = {
				{ {1,0}, Silica::MakeWidget<Silica::SBox>({
					.child = Silica::MakeWidget<Silica::SEditableText>({
						.initialText = outPath,
						.onTextChanged = [this, &outPath](const std::string& val) {
							outPath = val;
							validate();
						}
					})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = {8, 4},
					.onClick = [this, &outPath, typeDesc, filter, defaultAssetsSubDir]() {
						std::filesystem::path dir = ProjectManager::getProject()->getAssetsPath() / defaultAssetsSubDir;
						if (!std::filesystem::exists(dir)) dir = ProjectManager::getProject()->getAssetsPath();
						std::filesystem::path absPath = FileDialogs::openFile({ {typeDesc, filter} }, dir);
						if (!absPath.empty()) { outPath = absPath.string(); rebuildUI(); }
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse..." })
				})}
			}
		});
	}

	Silica::WidgetPtr ModalBase::makeDirectoryRow(std::string& outPath, const std::string& defaultAssetsSubDir) {
		return Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 8.0f,
			.slots = {
				{ {1,0}, Silica::MakeWidget<Silica::SBox>({
					.child = Silica::MakeWidget<Silica::SEditableText>({
						.initialText = outPath,
						.onTextChanged = [this, &outPath](const std::string& val) { outPath = val; validate(); }
					})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = {8, 4},
					.onClick = [this, &outPath, defaultAssetsSubDir]() {
						std::filesystem::path dir = ProjectManager::getProject()->getAssetsPath() / defaultAssetsSubDir;
						if (!std::filesystem::exists(dir)) dir = ProjectManager::getProject()->getAssetsPath();

						std::filesystem::path absPath = FileDialogs::openFolder(dir);
						if (!absPath.empty()) { outPath = absPath.string(); rebuildUI(); }
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse..." })
				})}
			}
		});
	}

}
