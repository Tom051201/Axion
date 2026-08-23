#include "studiopch.h"
#include "HistoryPanel.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SScrollBox.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SButton.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SVerticalBox.h>

#include "AxionStudio/Source/core/EditorCommand.h"

namespace Axion {

	void HistoryPanel::onEvent(Event& ev) {
		EventDispatcher dispatcher(ev);
		dispatcher.dispatch<EditorHistoryChangedEvent>(AX_BIND_EVENT_FN(onEditorHistoryChanged));
	}

	EventReply HistoryPanel::onEditorHistoryChanged(EditorHistoryChangedEvent& ev) {
		rebuildUI();
		return EventReply::unhandled();
	}

	Silica::WidgetPtr HistoryPanel::getWidget() {
		if (!m_uiRoot) {
			m_contentBox = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 1.0f });

			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.borderThickness = Silica::GetTheme().Border_Thickness,
				.backgroundColor = Silica::GetTheme().Background_Panel,
				.child = Silica::MakeWidget<Silica::SScrollBox>({
					.child = Silica::MakeWidget<Silica::SBox>({
						.padding = { 5.0f, 5.0f },
						.backgroundColor = Silica::Color::transparent(),
						.child = m_contentBox
					})
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
			.padding = {0, 0},
			.child = Silica::MakeWidget<Silica::SButton>({
				.padding = { 8.0f, 4.0f },
				.color = isBaseCurrent ? Silica::GetTheme().Accent_Primary : Silica::Color::transparent(),
				.hoverColor = Silica::GetTheme().Element_Hover,
				.onClick = [this]() {
					EditorCommandManager::jumpTo(0);
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({
					.text = "[ Original State ]",
					.color = Silica::GetTheme().Text_Dim
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
				.padding = {0, 0},
				.child = Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
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
