#include "studiopch.h"
#include "HistoryPanel.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SScrollBox.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SButton.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SBorderLayout.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SAlign.h>
#include <Silica/include/SMenuAnchor.h>
#include <Silica/include/SImage.h>
#include <Silica/include/SSpacer.h>

#include "AxionStudio/Source/core/EditorCommand.h"
#include "AxionStudio/Source/core/SilicaContext.h"
#include "AxionStudio/Source/ui/SilicaHelpers.h"
#include "AxionStudio/Source/ui/EditorTheme.h"

namespace {
	constexpr float ITEM_SPACING = 1.0f;
}

namespace Axion {

	void HistoryPanel::onEvent(Event& ev) {
		EventDispatcher dispatcher(ev);
		dispatcher.dispatch<EditorHistoryChangedEvent>(AX_BIND_EVENT_FN(HistoryPanel::onEditorHistoryChanged));
	}

	EventReply HistoryPanel::onEditorHistoryChanged(EditorHistoryChangedEvent& ev) {
		rebuildUI();
		return EventReply::unhandled();
	}

	Silica::WidgetPtr HistoryPanel::getWidget() {
		if (!m_uiRoot) {
			m_contentBox = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = ITEM_SPACING });

			// -- Options Menu --
			auto optionsMenu = Silica::MakeWidget<Silica::SAlign>({
				.verticalAlign = Silica::VerticalAlign::Center,
				.child = Silica::MakeWidget<Silica::SMenuAnchor>({
					.openOnHover = false,
					.openToRight = true,
					.anchorContent = Silica::MakeWidget<Silica::SButton>({
						.padding = { EditorTheme::PADDING_SMALL, EditorTheme::PADDING_SMALL },
						.color = Silica::Color::transparent(),
						.hoverColor = EditorTheme::BUTTON_COLOR_HOVER_SUBTLE,
						.onClick = []() { return Silica::EventReply::unhandled(); },
						.child = Silica::MakeWidget<Silica::SImage>({
							.textureID = SilicaContext::getIcon("GearIcon"),
							.desiredSize = { EditorTheme::ICON_SIZE_SMALL, EditorTheme::ICON_SIZE_SMALL }
						})
					}),
					.menuContent = Silica::MakeWidget<Silica::SBox>({
						.padding = { EditorTheme::PADDING_SMALL, EditorTheme::PADDING_SMALL },
						.explicitSize = Silica::Vec2{ EditorTheme::OPTIONS_MENU_WIDTH, 0.0f },
						.hasBorder = true,
						.backgroundColor = Silica::GetTheme().Background_Popup,
						.child = Silica::MakeWidget<Silica::SVerticalBox>({
							.spacing = EditorTheme::SPACING_SMALL,
							.slots = {
								{ {0,0}, SilicaHelpers::MakeOptionsMenuItem("Clear History", []() {
									// Placeholder for future clear logic
								}) }
							}
						})
					})
				})
			});

			// -- Toolbar --
			auto topBarBox = Silica::MakeWidget<Silica::SBox>({
				.padding = { EditorTheme::TOOLBAR_PADDING_X, 0.0f },
				.explicitSize = Silica::Vec2{ 0.0f, EditorTheme::TOOLBAR_HEIGHT },
				.backgroundColor = Silica::GetTheme().Surface_Tertiary,
				.child = Silica::MakeWidget<Silica::SHorizontalBox>({
					.spacing = EditorTheme::TOOLBAR_SPACING,
					.slots = {
						{ {0, 0}, optionsMenu },
						{ {0, 0}, Silica::MakeWidget<Silica::SSpacer>({.size = { EditorTheme::SPACING_LARGE, 0.0f } }) },
						{ {0, 0}, Silica::MakeWidget<Silica::SAlign>({
							.verticalAlign = Silica::VerticalAlign::Center,
							.child = Silica::MakeWidget<Silica::STextBlock>({.text = "History" })
						})}
					}
				})
			});

			// -- Scrollable Content --
			auto scrollContent = Silica::MakeWidget<Silica::SScrollBox>({
				.child = Silica::MakeWidget<Silica::SBox>({
					.padding = { EditorTheme::PADDING_MEDIUM, EditorTheme::PADDING_MEDIUM },
					.backgroundColor = Silica::Color::transparent(),
					.child = m_contentBox
				})
			});

			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.hasBorder = true,
				.child = Silica::MakeWidget<Silica::SBorderLayout>({
					.topBar = topBarBox,
					.contentArea = scrollContent
				})
			});

			rebuildUI();
		}
		return m_uiRoot;
	}

	void HistoryPanel::rebuildUI() {
		if (!m_contentBox) return;
		m_contentBox->clearSlots();

		const auto& history = EditorCommandManager::getHistory();
		size_t currentIndex = EditorCommandManager::getCurrentIndex();

		// -- Base State --
		bool isBaseCurrent = (currentIndex == 0);
		m_contentBox->addSlot({
			.child = Silica::MakeWidget<Silica::SButton>({
				.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
				.color = isBaseCurrent ? Silica::GetTheme().Accent_Primary : Silica::Color::transparent(),
				.onClick = []() {
					EditorCommandManager::jumpTo(0);
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({
					.text = "[ Original State ]",
					.color = isBaseCurrent ? Silica::GetTheme().Text_Main : Silica::GetTheme().Text_Success
				})
			})
		});

		// -- Command History --
		for (size_t i = 0; i < history.size(); ++i) {
			size_t jumpIndex = i + 1;

			bool isActive = (jumpIndex <= currentIndex);
			bool isCurrent = (jumpIndex == currentIndex);

			Silica::Color btnColor = isCurrent ? Silica::GetTheme().Accent_Primary : Silica::Color::transparent();
			Silica::Color txtColor = isActive ? Silica::GetTheme().Text_Main : Silica::GetTheme().Text_Dim;

			m_contentBox->addSlot({
				.child = Silica::MakeWidget<Silica::SButton>({
					.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
					.color = btnColor,
					.onClick = [jumpIndex]() {
						EditorCommandManager::jumpTo(jumpIndex);
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({
						.text = std::to_string(jumpIndex) + ": " + history[i]->getName(),
						.color = txtColor
					})
				})
			});
		}
	}

}
