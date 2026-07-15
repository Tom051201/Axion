#include "SettingsModal.h"

#include <stdexcept>

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionStudio/Source/core/EditorActionQueue.h"

#include "AxionStudio/Vendor/Silica/include/SBorderLayout.h"
#include "AxionStudio/Vendor/Silica/include/SHorizontalBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/SButton.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SAlign.h"
#include "AxionStudio/Vendor/Silica/include/SSliderFloat.h"
#include "AxionStudio/Vendor/Silica/include/SSeparator.h"
#include "AxionStudio/Vendor/Silica/include/SEditableText.h"
#include "AxionStudio/Vendor/Silica/include/Theme.h"


namespace Axion {

	Silica::WidgetPtr SettingsModal::getWidget(std::function<void()> onClose) {
		m_onClose = onClose;

		if (!m_uiRoot) {
			m_budgetText = std::to_string(AssetManager::getMaxAssetsPerFrame());

			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.backgroundColor = Silica::Color(0, 0, 0, 180),
			});
			rebuildUI_Internal();
		}
		return m_uiRoot;
	}

	void SettingsModal::rebuildUI() {
		if (m_rebuildQueued) return;
		m_rebuildQueued = true;

		EditorActionQueue::push([this]() {
			m_rebuildQueued = false;
			rebuildUI_Internal();
		});
	}

	void SettingsModal::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 15.0f });

		// -- Header --
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
			.text = "Editor Preferences",
			.color = Silica::GetTheme().Text_Main
		}) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({}) });


		// -- Asset Loading Input Field --
		auto inputRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 15.0f,
			.slots = {
				{ {0,0}, Silica::MakeWidget<Silica::SBox>({
					.explicitSize = Silica::Vec2{ 200.0f, 0.0f },
					.backgroundColor = Silica::Color::transparent(),
					.child = Silica::MakeWidget<Silica::SAlign>({
						.verticalAlign = Silica::VerticalAlign::Center,
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Asset Load Budget (Per Frame):" })
					})
				})},

					// --- NEW: Editable Text with safe parsing validation ---
					{ {1,0}, Silica::MakeWidget<Silica::SBox>({
						.child = Silica::MakeWidget<Silica::SEditableText>({
							.initialText = m_budgetText,
							.onTextChanged = [this](const std::string& val) {
								m_budgetText = val;

								try {
									int parsed = std::stoi(val);
									if (parsed >= 1) {
										AssetManager::setMaxAssetsPerFrame((uint32_t)parsed);
									}
								}
								catch (...) {
									// Value was empty or contained letters. 
									// We ignore it and wait for them to type a valid number!
								}
							}
						})
					})}
				}
			});

		contentBox->addSlot({ {0,0}, inputRow });

		// Optional: Add some spacing before the footer
		contentBox->addSlot({ {1,0}, Silica::MakeWidget<Silica::SBox>({.backgroundColor = Silica::Color::transparent() }) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({}) });


		// -- Footer (Done Button) --
		auto doneBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = { 20.0f, 8.0f },
			.hoverColor = Silica::GetTheme().Accent_Primary,
			.onClick = [this]() {
				if (m_onClose) m_onClose();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Done" })
			});

		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SAlign>({
			.horizontalAlign = Silica::HorizontalAlign::Right,
			.child = doneBtn
		}) });

		// -- Assemble Modal --
		auto modalPanel = Silica::MakeWidget<Silica::SBox>({
			.padding = { 20.0f, 20.0f },
			.explicitSize = Silica::Vec2{ 500.0f, 0.0f },
			.borderThickness = Silica::GetTheme().Border_Thickness,
			.backgroundColor = Silica::GetTheme().Background_Panel,
			.child = contentBox
		});

		m_uiRoot->setChild(Silica::MakeWidget<Silica::SAlign>({
			.horizontalAlign = Silica::HorizontalAlign::Center,
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = modalPanel
		}));
	}

}
