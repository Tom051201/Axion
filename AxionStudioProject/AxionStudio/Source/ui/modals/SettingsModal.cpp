#include "studiopch.h"
#include "SettingsModal.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SBorderLayout.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SButton.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SAlign.h>
#include <Silica/include/SSliderFloat.h>
#include <Silica/include/SSeparator.h>
#include <Silica/include/SEditableText.h>
#include <Silica/include/SScrollBox.h>

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/PlatformUtils.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"

namespace Axion {

	Silica::WidgetPtr SettingsModal::getWidget(const std::vector<std::string>& currentPaths, std::function<void(std::vector<std::string>)> onApply, std::function<void()> onClose) {
		m_onApply = onApply;
		m_onClose = onClose;

		if (!m_uiRoot) {
			m_budgetText = std::to_string(AssetManager::getMaxAssetsPerFrame());
			m_libraryPaths = currentPaths;

			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.consumePointerEvents = true,
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

	Silica::WidgetPtr SettingsModal::buildEditorPreferencesTab() {
		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 15.0f });

		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Editor Preferences" }) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({}) });

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
							catch (...) {}
						}
					})
				})}
			}
		});

		contentBox->addSlot({ {0,0}, inputRow });
		return contentBox;
	}

	Silica::WidgetPtr SettingsModal::buildFilePathsTab() {
		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 15.0f });

		// -- Header Row --
		auto headerRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.slots = {
				{ {1,0}, Silica::MakeWidget<Silica::SAlign>({
					.verticalAlign = Silica::VerticalAlign::Center,
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Asset Library Search Paths" })
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 12.0f, 4.0f },
					.color = Silica::Color::transparent(),
					.hoverColor = Silica::GetTheme().Accent_Primary,
					.onClick = [this]() {
						std::filesystem::path newPath = FileDialogs::openFolder(std::filesystem::current_path());
						if (!newPath.empty()) {
							m_libraryPaths.push_back(newPath.string());
							rebuildUI();
						}
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "+"})
				})}
			}
			});

		contentBox->addSlot({ {0,0}, headerRow });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({}) });

		// -- List of Paths --
		auto pathsList = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 5.0f });

		for (size_t i = 0; i < m_libraryPaths.size(); ++i) {
			auto pathRow = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 10.0f,
				.slots = {
					{ {1,0}, Silica::MakeWidget<Silica::SBox>({
						.child = Silica::MakeWidget<Silica::SEditableText>({
							.initialText = m_libraryPaths[i],
							.onTextChanged = [this, i](const std::string& val) {
								m_libraryPaths[i] = val;
							}
						})
					})},
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = { 10.0f, 6.0f },
						.color = Silica::GetTheme().Accent_Danger,
						.onClick = [this, i]() {
							m_libraryPaths.erase(m_libraryPaths.begin() + i);
							rebuildUI();
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "X"})
					})}
				}
				});
			pathsList->addSlot({ {0,0}, pathRow });
		}

		contentBox->addSlot({ {1,0}, Silica::MakeWidget<Silica::SScrollBox>({.child = pathsList }) });

		return contentBox;
	}

	void SettingsModal::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		// -- Sidebar Tab Button Generator --
		auto makeTabButton = [this](const std::string& label, Tab targetTab) {
			bool isActive = (m_activeTab == targetTab);
			return Silica::MakeWidget<Silica::SButton>({
				.padding = { 15.0f, 10.0f },
				.color = isActive ? Silica::GetTheme().Surface_Tertiary : Silica::Color::transparent(),
				.onClick = [this, targetTab]() {
					m_activeTab = targetTab;
					rebuildUI();
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({
					.text = label,
					.color = isActive ? Silica::GetTheme().Text_Main : Silica::GetTheme().Text_Dim
				})
			});
		};

		// -- Left Sidebar --
		auto sidebar = Silica::MakeWidget<Silica::SBox>({
			.explicitSize = Silica::Vec2{ 140.0f, 0.0f },
			.backgroundColor = Silica::GetTheme().Surface_Secondary,
			.child = Silica::MakeWidget<Silica::SVerticalBox>({
				.spacing = 2.0f,
				.slots = {
					{ {0,0}, makeTabButton("Preferences", Tab::EditorPreferences) },
					{ {0,0}, makeTabButton("File Paths", Tab::FilePaths) }
				}
			})
		});

		// -- Right Content Area --
		Silica::WidgetPtr activeContent = nullptr;
		if (m_activeTab == Tab::EditorPreferences) activeContent = buildEditorPreferencesTab();
		else if (m_activeTab == Tab::FilePaths) activeContent = buildFilePathsTab();

		auto contentArea = Silica::MakeWidget<Silica::SBox>({
			.padding = { 20.0f, 20.0f },
			.backgroundColor = Silica::Color::transparent(),
			.child = activeContent
		});

		// -- Split Layout --
		auto splitLayout = Silica::MakeWidget<Silica::SHorizontalBox>({
			.slots = {
				{ {0,0}, sidebar },
				{ {1,0}, contentArea }
			}
		});

		auto topSection = Silica::MakeWidget<Silica::SBox>({
			.explicitSize = Silica::Vec2{ 850.0f, 534.0f },
			.child = splitLayout
		});

		// -- Footer --
		auto footerBox = Silica::MakeWidget<Silica::SBox>({
			.padding = { 20.0f, 15.0f },
			.explicitSize = Silica::Vec2{ 850.0f, 66.0f },
			.backgroundColor = Silica::GetTheme().Surface_Secondary,
			.child = Silica::MakeWidget<Silica::SAlign>({
				.horizontalAlign = Silica::HorizontalAlign::Right,
				.verticalAlign = Silica::VerticalAlign::Center,
				.child = Silica::MakeWidget<Silica::SButton>({
					.padding = { 30.0f, 8.0f },
					.hoverColor = Silica::GetTheme().Accent_Primary,
					.onClick = [this]() {
						if (m_onApply) m_onApply(m_libraryPaths);

						if (m_onClose) m_onClose();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Done" })
				})
			})
		});

		// -- Stack Top Section And Footer Cleanly --
		auto fullLayout = Silica::MakeWidget<Silica::SVerticalBox>({
			.slots = {
				{ {0,0}, topSection },
				{ {0,0}, footerBox }
			}
		});

		// -- Assemble Modal --
		auto modalPanel = Silica::MakeWidget<Silica::SBox>({
			.explicitSize = Silica::Vec2{ 850.0f, 600.0f },
			.borderThickness = Silica::GetTheme().Border_Thickness,
			.child = fullLayout
		});

		m_uiRoot->setChild(Silica::MakeWidget<Silica::SAlign>({
			.horizontalAlign = Silica::HorizontalAlign::Center,
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = modalPanel
		}));
	}

}
