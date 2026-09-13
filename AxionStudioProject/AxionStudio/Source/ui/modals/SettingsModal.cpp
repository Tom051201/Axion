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
#include <Silica/include/SCheckbox.h>
#include <Silica/include/SInputFieldInt.h>

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/PlatformUtils.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/ui/SilicaHelpers.h"

namespace {
	constexpr float MODAL_WIDTH = 1250.0f;
	constexpr float MODAL_HEIGHT = 700.0f;
	constexpr float FOOTER_HEIGHT = 66.0f;
	constexpr float TOP_SECTION_HEIGHT = MODAL_HEIGHT - FOOTER_HEIGHT;

	constexpr float SIDEBAR_WIDTH = 160.0f;
	constexpr float LABEL_WIDTH = 220.0f;

	constexpr float SPACING_LARGE = 15.0f;
	constexpr float SPACING_SMALL = 5.0f;
	constexpr float PADDING_LARGE = 20.0f;
}

namespace Axion {

	Silica::WidgetPtr SettingsModal::getWidget(const SettingsPayload& initialSettings, std::function<void(const SettingsPayload&)> onApply, std::function<void()> onClose) {
		m_onApply = onApply;
		m_onClose = onClose;

		if (!m_uiRoot) {
			m_workingSettings = initialSettings;

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
		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = SPACING_LARGE });

		contentBox->addSlot({ {0,0}, SilicaHelpers::MakeHeader("Editor Preferences") });

		// -- Asset load budget --
		auto budgetInput = Silica::MakeWidget<Silica::SBox>({
			.child = Silica::MakeWidget<Silica::SInputFieldInt>({
				.initialValue = (int)m_workingSettings.maxAssetsPerFrame,
				.onValueChanged = [this](int val) {
					m_workingSettings.maxAssetsPerFrame = static_cast<uint32_t>(std::max(1, val));
				}
			})
		});
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Asset Load Budget (Per Frame):", budgetInput, LABEL_WIDTH) });

		// -- Discord RPC --
		auto discordCheck = Silica::MakeWidget<Silica::SCheckBox>({
			.initialCheck = m_workingSettings.enableDiscordRPC,
			.onCheckChanged = [this](bool val) {
				m_workingSettings.enableDiscordRPC = val;
			}
		});
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Enable Discord Rich Presence:", discordCheck, LABEL_WIDTH) });

		return contentBox;
	}

	Silica::WidgetPtr SettingsModal::buildFilePathsTab() {
		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = SPACING_LARGE });

		// -- Header Row --
		auto headerRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.slots = {
				{ {1,0}, Silica::MakeWidget<Silica::SAlign>({
					.verticalAlign = Silica::VerticalAlign::Center,
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Asset Library Search Paths", .color = Silica::GetTheme().Accent_Primary })
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 12.0f, 4.0f },
					.color = Silica::Color::transparent(),
					.hoverColor = Silica::GetTheme().Accent_Primary,
					.onClick = [this]() {
						std::filesystem::path newPath = FileDialogs::openFolder(std::filesystem::current_path());
						if (!newPath.empty()) {
							m_workingSettings.assetLibraryPaths.push_back(newPath.string());
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
		auto pathsList = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = SPACING_SMALL });

		for (size_t i = 0; i < m_workingSettings.assetLibraryPaths.size(); ++i) {
			auto pathRow = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 10.0f,
				.slots = {
					{ {1,0}, Silica::MakeWidget<Silica::SBox>({
						.child = Silica::MakeWidget<Silica::SEditableText>({
							.initialText = m_workingSettings.assetLibraryPaths[i],
							.onTextChanged = [this, i](const std::string& val) {
								m_workingSettings.assetLibraryPaths[i] = val;
							}
						})
					})},
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = { 10.0f, 6.0f },
						.color = Silica::GetTheme().Accent_Danger,
						.onClick = [this, i]() {
							m_workingSettings.assetLibraryPaths.erase(m_workingSettings.assetLibraryPaths.begin() + i);
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

	Silica::WidgetPtr SettingsModal::buildPanelsTab() {
		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = SPACING_LARGE });

		// -- Content Browser --
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakeHeader("Content Browser") });

		auto cbShowContentArea = Silica::MakeWidget<Silica::SCheckBox>({
			.initialCheck = m_workingSettings.contentBrowserShowContentArea,
			.onCheckChanged = [this](bool val) { m_workingSettings.contentBrowserShowContentArea = val; }
		});
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Show Content Area:", cbShowContentArea, LABEL_WIDTH) });

		auto cbShowVFSTree = Silica::MakeWidget<Silica::SCheckBox>({
			.initialCheck = m_workingSettings.contentBrowserShowVFSTree,
			.onCheckChanged = [this](bool val) { m_workingSettings.contentBrowserShowVFSTree = val; }
		});
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Show Collection Tree:", cbShowVFSTree, LABEL_WIDTH) });

		auto cbShowPhysicalTree = Silica::MakeWidget<Silica::SCheckBox>({
			.initialCheck = m_workingSettings.contentBrowserShowPhysicalTree,
			.onCheckChanged = [this](bool val) { m_workingSettings.contentBrowserShowPhysicalTree = val; }
		});
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Show Physical Tree:", cbShowPhysicalTree, LABEL_WIDTH) });


		// -- Material Editor --
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakeHeader("Material Editor") });

		auto cbInvertCam = Silica::MakeWidget<Silica::SCheckBox>({
			.initialCheck = m_workingSettings.materialEditorInvertCamera,
			.onCheckChanged = [this](bool val) { m_workingSettings.materialEditorInvertCamera = val; }
		});
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Invert Camera Pan/Orbit:", cbInvertCam, LABEL_WIDTH) });

		return contentBox;
	}

	void SettingsModal::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		// -- Sidebar Tab Button Generator --
		auto makeTabButton = [this](const std::string& label, Tab targetTab) {
			bool isActive = (m_activeTab == targetTab);
			return Silica::MakeWidget<Silica::SButton>({
				.padding = { SPACING_LARGE, 10.0f },
				.color = isActive ? Silica::GetTheme().Accent_Primary : Silica::Color::transparent(),
				.hoverColor = isActive ? Silica::GetTheme().Accent_Primary : Silica::GetTheme().Element_Hover,
				.pressedColor = isActive ? Silica::GetTheme().Accent_Primary : Silica::GetTheme().Element_Pressed,
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
			.explicitSize = Silica::Vec2{ SIDEBAR_WIDTH, 0.0f },
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.child = Silica::MakeWidget<Silica::SVerticalBox>({
				.slots = {
					{ {0,0}, makeTabButton("Preferences", Tab::EditorPreferences) },
					{ {0,0}, makeTabButton("File Paths", Tab::FilePaths) },
					{ {0,0}, makeTabButton("Panels", Tab::Panels) },
				}
			})
		});

		// -- Right Content Area --
		Silica::WidgetPtr activeContent = nullptr;
		if (m_activeTab == Tab::EditorPreferences) activeContent = buildEditorPreferencesTab();
		else if (m_activeTab == Tab::FilePaths) activeContent = buildFilePathsTab();
		else if (m_activeTab == Tab::Panels) activeContent = buildPanelsTab();

		auto contentArea = Silica::MakeWidget<Silica::SBox>({
			.padding = { PADDING_LARGE, PADDING_LARGE },
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
			.explicitSize = Silica::Vec2{ MODAL_WIDTH, TOP_SECTION_HEIGHT },
			.child = splitLayout
		});

		// -- Footer --
		auto footerBox = Silica::MakeWidget<Silica::SBox>({
			.padding = { PADDING_LARGE, SPACING_LARGE },
			.explicitSize = Silica::Vec2{ MODAL_WIDTH, FOOTER_HEIGHT },
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.child = Silica::MakeWidget<Silica::SAlign>({
				.horizontalAlign = Silica::HorizontalAlign::Right,
				.verticalAlign = Silica::VerticalAlign::Center,
				.child = Silica::MakeWidget<Silica::SButton>({
					.padding = { 30.0f, 8.0f },
					.hoverColor = Silica::GetTheme().Accent_Primary,
					.onClick = [this]() {
						if (m_onApply) m_onApply(m_workingSettings);

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
			.explicitSize = Silica::Vec2{ MODAL_WIDTH, MODAL_HEIGHT },
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
