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
#include <Silica/include/SOverlay.h>
#include <Silica/include/SWrappedTextBlock.h>
#include <Silica/include/SComboBox.h>

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/graphics/Renderer.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/EditorEvents.h"
#include "AxionStudio/Source/core/EditorSettings.h"
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

	Silica::WidgetPtr SettingsModal::getWidget(std::function<void()> onClose) {
		m_onClose = onClose;
		m_pendingChanges = EditorSettingType::None;
		m_showCancelPopup = false;

		m_workingState.maxAssetsPerFrame = AssetManager::getMaxAssetsPerFrame();
		m_workingState.enableDiscordRPC = EditorSettings::enableDiscordRPC;
		m_workingState.assetLibraryPaths = EditorSettings::assetLibraryPaths;
		m_workingState.contentBrowserShowContentArea = EditorSettings::contentBrowserShowContentArea;
		m_workingState.contentBrowserShowVFSTree = EditorSettings::contentBrowserShowVFSTree;
		m_workingState.contentBrowserShowPhysicalTree = EditorSettings::contentBrowserShowPhysicalTree;
		m_workingState.materialEditorInvertCamera = EditorSettings::materialEditorInvertCamera;
		m_workingState.viewportPanelShowRendererStats = EditorSettings::viewportPanelShowRendererStats;
		m_workingState.viewportPanelInvertCameraX = EditorSettings::viewportPanelInvertCameraX;
		m_workingState.viewportPanelInvertCameraY = EditorSettings::viewportPanelInvertCameraY;
		m_workingState.viewportPanelGizmoScale = EditorSettings::viewportPanelGizmoScale;

		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.consumePointerEvents = true,
				.backgroundColor = Silica::Color(0, 0, 0, 180), // TODO: add those to the editor Theme
			});
		}
		rebuildUI_Internal();

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
		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = SPACING_LARGE });
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakeHeader("Editor Preferences") });

		// -- Asset load budget --
		auto budgetInput = Silica::MakeWidget<Silica::SBox>({
			.child = Silica::MakeWidget<Silica::SInputFieldInt>({
				.initialValue = (int)m_workingState.maxAssetsPerFrame,
				.onValueChanged = [this](int val) {
					m_workingState.maxAssetsPerFrame = static_cast<uint32_t>(std::max(1, val));
					m_pendingChanges |= EditorSettingType::AssetLoadBudget;
				}
			})
		});
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Asset Load Budget (Per Frame):", budgetInput, LABEL_WIDTH) });

		// -- Discord RPC --
		auto discordCheck = Silica::MakeWidget<Silica::SCheckBox>({
			.initialCheck = m_workingState.enableDiscordRPC,
			.onCheckChanged = [this](bool val) {
				m_workingState.enableDiscordRPC = val;
				m_pendingChanges |= EditorSettingType::DiscordRPC;
			}
		});
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Enable Discord Rich Presence:", discordCheck, LABEL_WIDTH) });

		return contentBox;
	}

	Silica::WidgetPtr SettingsModal::buildGraphicsTab() {
		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = SPACING_LARGE });
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakeHeader("Graphics") });

		// -- Graphics API --
		std::vector<std::string> apiOptions = { "DirectX 11", "DirectX 12" };
		std::string currentApi = "DirectX 12";
		if (Renderer::getAPI() == RendererAPI::DirectX11) { currentApi = "DirectX 11"; }
		auto graphicsAPIInput = Silica::MakeWidget<Silica::SComboBox>({
			.options = apiOptions,
			.initialValue = currentApi,
			.onValueChanged = [](const std::string& val) {
				// TODO: implement this entirely
			}
		});
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Graphics API:", graphicsAPIInput, LABEL_WIDTH) });

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
							m_workingState.assetLibraryPaths.push_back(newPath.string());
							m_pendingChanges |= EditorSettingType::AssetLibraryPaths;
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

		for (size_t i = 0; i < m_workingState.assetLibraryPaths.size(); i++) {
			auto pathRow = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 10.0f,
				.slots = {
					{ {1,0}, Silica::MakeWidget<Silica::SBox>({
						.child = Silica::MakeWidget<Silica::SEditableText>({
							.initialText = m_workingState.assetLibraryPaths[i],
							.onTextChanged = [this, i](const std::string& val) {
								m_workingState.assetLibraryPaths[i] = val;
								m_pendingChanges |= EditorSettingType::AssetLibraryPaths;
							}
						})
					})},
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = { 10.0f, 6.0f },
						.color = Silica::GetTheme().Accent_Danger,
						.onClick = [this, i]() {
							m_workingState.assetLibraryPaths.erase(m_workingState.assetLibraryPaths.begin() + i);
							m_pendingChanges |= EditorSettingType::AssetLibraryPaths;
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
			.initialCheck = m_workingState.contentBrowserShowContentArea,
			.onCheckChanged = [this](bool val) {
				m_workingState.contentBrowserShowContentArea = val;
				m_pendingChanges |= EditorSettingType::ContentBrowserLayout;
			}
		});
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Show Content Area:", cbShowContentArea, LABEL_WIDTH) });

		auto cbShowVFSTree = Silica::MakeWidget<Silica::SCheckBox>({
			.initialCheck = m_workingState.contentBrowserShowVFSTree,
			.onCheckChanged = [this](bool val) {
				m_workingState.contentBrowserShowVFSTree = val;
				m_pendingChanges |= EditorSettingType::ContentBrowserLayout;
			}
		});
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Show Collection Tree:", cbShowVFSTree, LABEL_WIDTH) });

		auto cbShowPhysicalTree = Silica::MakeWidget<Silica::SCheckBox>({
			.initialCheck = m_workingState.contentBrowserShowPhysicalTree,
			.onCheckChanged = [this](bool val) {
				m_workingState.contentBrowserShowPhysicalTree = val;
				m_pendingChanges |= EditorSettingType::ContentBrowserLayout;
			}
		});
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Show Physical Tree:", cbShowPhysicalTree, LABEL_WIDTH) });


		// -- Material Editor --
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakeHeader("Material Editor") });

		auto cbInvertCam = Silica::MakeWidget<Silica::SCheckBox>({
			.initialCheck = m_workingState.materialEditorInvertCamera,
			.onCheckChanged = [this](bool val) {
				m_workingState.materialEditorInvertCamera = val;
				m_pendingChanges |= EditorSettingType::MaterialEditorCamera;
			}
		});
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Invert Camera Pan/Orbit:", cbInvertCam, LABEL_WIDTH) });

		// -- Viewport Panel --
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakeHeader("Viewport") });

		auto vpShowStats = Silica::MakeWidget<Silica::SCheckBox>({
			.initialCheck = m_workingState.viewportPanelShowRendererStats,
			.onCheckChanged = [this](bool val) {
				m_workingState.viewportPanelShowRendererStats = val;
				m_pendingChanges |= EditorSettingType::Viewport;
			}
		});
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Show Renderer Stats:", vpShowStats, LABEL_WIDTH) });

		auto vpInverCamX = Silica::MakeWidget<Silica::SCheckBox>({
			.initialCheck = m_workingState.viewportPanelInvertCameraX,
			.onCheckChanged = [this](bool val) {
				m_workingState.viewportPanelInvertCameraX = val;
				m_pendingChanges |= EditorSettingType::Viewport;
			}
		});
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Invert Camera X-Axis:", vpInverCamX, LABEL_WIDTH) });

		auto vpInverCamY = Silica::MakeWidget<Silica::SCheckBox>({
			.initialCheck = m_workingState.viewportPanelInvertCameraY,
			.onCheckChanged = [this](bool val) {
				m_workingState.viewportPanelInvertCameraY = val;
				m_pendingChanges |= EditorSettingType::Viewport;
			}
		});
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Invert Camera Y-Axis:", vpInverCamY, LABEL_WIDTH) });

		auto vpGizmoScale = Silica::MakeWidget<Silica::SSliderFloat>({
			.initialValue = EditorSettings::viewportPanelGizmoScale,
			.minValue = 0.1f,
			.maxValue = 3.0f,
			.snapStep = 0.1f,
			.onValueChanged = [this](float val) {
				m_workingState.viewportPanelGizmoScale = val;
				m_pendingChanges |= EditorSettingType::Viewport;
			}
		});
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Gizmo Scale:", vpGizmoScale, LABEL_WIDTH) });

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
					{ {0,0}, makeTabButton("Graphics", Tab::Graphics) },
					{ {0,0}, makeTabButton("File Paths", Tab::FilePaths) },
					{ {0,0}, makeTabButton("Panels", Tab::Panels) },
				}
			})
		});

		// -- Right Content Area --
		Silica::WidgetPtr activeContent = nullptr;
		if (m_activeTab == Tab::EditorPreferences) activeContent = buildEditorPreferencesTab();
		else if (m_activeTab == Tab::Graphics) activeContent = buildGraphicsTab();
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

		// -- Apply Logic --
		auto applyChanges = [this]() {
			// -- Commit values --
			AssetManager::setMaxAssetsPerFrame(m_workingState.maxAssetsPerFrame);
			EditorSettings::enableDiscordRPC = m_workingState.enableDiscordRPC;
			EditorSettings::assetLibraryPaths = m_workingState.assetLibraryPaths;
			EditorSettings::contentBrowserShowContentArea = m_workingState.contentBrowserShowContentArea;
			EditorSettings::contentBrowserShowVFSTree = m_workingState.contentBrowserShowVFSTree;
			EditorSettings::contentBrowserShowPhysicalTree = m_workingState.contentBrowserShowPhysicalTree;
			EditorSettings::materialEditorInvertCamera = m_workingState.materialEditorInvertCamera;
			EditorSettings::viewportPanelShowRendererStats = m_workingState.viewportPanelShowRendererStats;
			EditorSettings::viewportPanelInvertCameraX = m_workingState.viewportPanelInvertCameraX;
			EditorSettings::viewportPanelInvertCameraY = m_workingState.viewportPanelInvertCameraY;
			EditorSettings::viewportPanelGizmoScale = m_workingState.viewportPanelGizmoScale;

			// -- Dispatch events --
			if (m_pendingChanges != EditorSettingType::None && m_eventCallback) {
				EditorSettingsChangedEvent e(m_pendingChanges);
				m_eventCallback(e);
			}

			m_pendingChanges = EditorSettingType::None;
		};

		// -- Action Buttons --
		auto actionButtons = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 10.0f,
			.slots = {
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 20.0f, 8.0f },
					.hoverColor = Silica::GetTheme().Accent_Danger,
					.onClick = [this]() {
						if (m_pendingChanges != EditorSettingType::None) {
							m_showCancelPopup = true;
							rebuildUI();
						}
						else {
							if (m_onClose) m_onClose();
						}
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Cancel" })
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 20.0f, 8.0f },
					.hoverColor = Silica::GetTheme().Accent_Primary,
					.onClick = [this, applyChanges]() {
						applyChanges();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Apply" })
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 30.0f, 8.0f },
					.hoverColor = Silica::GetTheme().Accent_Primary,
					.onClick = [this, applyChanges]() {
						applyChanges();
						if (m_onClose) m_onClose();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Save" })
				})}
			}
		});

		// -- Footer --
		auto footerBox = Silica::MakeWidget<Silica::SBox>({
			.padding = { PADDING_LARGE, SPACING_LARGE },
			.explicitSize = Silica::Vec2{ MODAL_WIDTH, FOOTER_HEIGHT },
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.child = Silica::MakeWidget<Silica::SAlign>({
				.horizontalAlign = Silica::HorizontalAlign::Right,
				.verticalAlign = Silica::VerticalAlign::Center,
				.child = actionButtons
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
			.hasBorder = true,
			.child = fullLayout
		});

		std::vector<Silica::WidgetPtr> overlayChildren;

		overlayChildren.push_back(Silica::MakeWidget<Silica::SAlign>({
			.horizontalAlign = Silica::HorizontalAlign::Center,
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = modalPanel
		}));

		// -- Validation Popup --
		if (m_showCancelPopup) {
			auto popupContent = Silica::MakeWidget<Silica::SVerticalBox>({
				.spacing = 20.0f,
				.slots = {
					{ {0,0}, SilicaHelpers::MakeHeader("Unsaved Changes") },
					{ {0,0}, Silica::MakeWidget<Silica::SWrappedTextBlock>({
						.text = "You have unsaved changes. Are you sure you want to discard them?",
						.wrapWidth = 400.0f
					}) },
					{ {0,0}, Silica::MakeWidget<Silica::SHorizontalBox>({
						.spacing = 10.0f,
						.slots = {
							{ {1,0}, Silica::MakeWidget<Silica::SBox>({}) },
							{ {0,0}, Silica::MakeWidget<Silica::SButton>({
								.padding = { 15.0f, 8.0f },
								.onClick = [this]() {
									m_showCancelPopup = false;
									rebuildUI();
									return Silica::EventReply::handled();
								},
								.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Keep Editing"})
							})},
							{ {0,0}, Silica::MakeWidget<Silica::SButton>({
								.padding = { 15.0f, 8.0f },
								.hoverColor = Silica::GetTheme().Accent_Danger,
								.onClick = [this]() {
									m_showCancelPopup = false;
									m_pendingChanges = EditorSettingType::None;
									if (m_onClose) m_onClose();
									return Silica::EventReply::handled();
								},
								.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Discard Changes"})
							})}
						}
					})}
				}
			});

			overlayChildren.push_back(Silica::MakeWidget<Silica::SBox>({
				.consumePointerEvents = true,
				.backgroundColor = Silica::Color(0, 0, 0, 180),
				.child = Silica::MakeWidget<Silica::SAlign>({
					.horizontalAlign = Silica::HorizontalAlign::Center,
					.verticalAlign = Silica::VerticalAlign::Center,
					.child = Silica::MakeWidget<Silica::SBox>({
						.padding = { 20.0f, 20.0f },
						.explicitSize = Silica::Vec2{ 450.0f, 0.0f },
						.hasBorder = true,
						.backgroundColor = Silica::GetTheme().Background_Popup,
						.child = popupContent
					})
				})
			}));
		}

		m_uiRoot->setChild(Silica::MakeWidget<Silica::SOverlay>({.children = overlayChildren }));
	}

}
